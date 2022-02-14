using Microsoft.Azure.EventHubs;
using Microsoft.Azure.WebJobs;
using Microsoft.Extensions.Logging;
using Receive.Models;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using IoTHubTrigger = Microsoft.Azure.WebJobs.EventHubTriggerAttribute;

[assembly: InternalsVisibleTo("Receive.Tests")]
namespace Receive
{

    public static class IoTHubData
    {

        [FunctionName("IoTHubData")]
        public static async Task Run(
            [IoTHubTrigger("messages/events", Connection = "AzureIoTHubConnectionString", ConsumerGroup = "$Default")] EventData message,
            [Table("Kineis", Connection = "AzureWebJobsStorage")] ICollector<TelemetryOutput> outputTable,
            Binder binder, ILogger log)
        {
            // Get incoming data packet
            var payload = Encoding.UTF8.GetString(message.Body.Array);
            var deviceId = message.SystemProperties["iothub-connection-device-id"];
            log.LogInformation($"IoT Hub trigger function processed a message: {payload} from {deviceId}");

            // Store raw information to blob storage
            var output = payload.ToCharArray();
            var filename = $"kineis/{DateTime.UtcNow:yyyy-MM-ddTHH-mm-ssZ}-{Guid.NewGuid().ToString().Substring(0,4)}.txt";
            var attribute = new BlobAttribute(filename, FileAccess.Write)
            {
                Connection = "AzureWebJobsStorage"
            };
            await using (var writer = await binder.BindAsync<TextWriter>(attribute))
            {
                await writer.WriteAsync(output, 0, output.Length);
            }

            // Unpack and interpret kineis data package
            var result = JsonSerializer.Deserialize<KineisRoot>(payload);
            foreach (var data in result.Data)
            {
                var parsedData = ParseKineisData(data.RawData);
                log.LogInformation($"Received raw data {data.RawData} which converted to {parsedData.Converted}, Id: {parsedData.Id}, Temperature: {parsedData.Temperature}, IsValid: {parsedData.IsValid}");
                if (parsedData.IsValid)
                {
                    // Store business data to azure table storage
                    try {
                        outputTable.Add(new TelemetryOutput { PartitionKey = "Temperature2", RowKey = parsedData.Id.ToString(), Message = parsedData.Temperature.ToString() });
                    } catch (Exception exception) {
                        log.LogWarning(exception, "Failed to save temperature reading. Does rowid already exist?");
                    }
                }
            }
        }

        internal static TelemetryResult ParseKineisData(string data)
        {
            List<string> hexValues = new List<string>();
            // Convert to pairs of hex from 18AFEA to 18 AF EA etc
            for (int i = 0; i < data.Length; i = i + 2)
            {
                hexValues.Add(data.Substring(i, 2));
            }
            // Convert to numbers from 18 AF EA to 24 175 234
            List<byte> bytes = new List<byte>();
            foreach (var hexValue in hexValues)
            {
                bytes.Add(byte.Parse(hexValue, System.Globalization.NumberStyles.HexNumber));
            }
            // Convert to chars from 24 175 234 to ascii characters
            var convertedString = string.Join("", bytes.Select(a => (char)a));

            // TODO: Deal with rubbish data coming through
            // Extra user data in format |45|14.6C (ID = 45, Temperature = 14.6C
            var match = Regex.Match(convertedString, @"\|([0-9]{1,3})\|([0-9.]{1,5})C");
            var result = new TelemetryResult
            {
                Converted = convertedString
            };

            if (match.Groups.Count == 3)
            {
                result.Id = int.Parse(match.Groups[1].Value);
                result.Temperature = double.Parse(match.Groups[2].Value);
            }
            // TODO: Extract day and time:
            // Skip 23 bits
            //DAT_DAY_1 5 bits = [1-31] days
            result.Day = ExtractNumberFromBits(bytes, 24, 5);
            //DAT_HOUR_1 5 bits = [0-23] hours
            result.Hour = ExtractNumberFromBits(bytes, 29, 5);
            //DAT_MIN_1 6 bits = [0-59] minutes
            result.Minute = ExtractNumberFromBits(bytes, 34, 6);
            return result;
        }

        internal static byte ExtractNumberFromBits(List<byte> bytes, int startBit, int numberOfBits)
        {
            var bitArray = bytes.ToBitArray(bytes.Count * 8);
            var result = new byte();
            int bitNumber = 0;
            if (numberOfBits > 0)
            {
                for (int i = startBit; i <= startBit + numberOfBits - 1; i++)
                {
                    result = (byte)(result & ~(1 << bitNumber) | ((bitArray[i] ? 1 : 0) << bitNumber));
                    bitNumber++;
                }
            }
            return result;
        }

        internal static BitArray ToBitArray(this List<byte> bytes, int bitCount)
        {
            BitArray ba = new BitArray(bitCount);
            for (int i = 0; i < bitCount; ++i)
            {
                ba.Set(i, ((bytes[i / 8] >> (i % 8)) & 0x01) > 0);
            }
            return ba;
        }
    }

}