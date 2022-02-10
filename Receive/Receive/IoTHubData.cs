using Microsoft.Azure.EventHubs;
using Microsoft.Azure.WebJobs;
using Microsoft.Extensions.Logging;
using Receive.Models;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using IoTHubTrigger = Microsoft.Azure.WebJobs.EventHubTriggerAttribute;
using System.Runtime.CompilerServices;
using System.Text.RegularExpressions;

[assembly: InternalsVisibleTo("Receive.Tests")]
namespace Receive
{
    public static class IoTHubData
    {

        [FunctionName("IoTHubData")]
        [Singleton] // To avoid collisions naming output files
        public static async Task Run(
            [IoTHubTrigger("messages/events", Connection = "AzureIoTHubConnectionString", ConsumerGroup = "$Default")] EventData message, 
            [Blob("kineis/{sys.utcnow}.txt", FileAccess.Write, Connection = "AzureWebJobsStorage")] Stream outputFile,
            [Table("Kineis", Connection = "AzureWebJobsStorage")] ICollector<TelemetryOutput> outputTable,
            ILogger log)
        {
            var payload = Encoding.UTF8.GetString(message.Body.Array);
            var deviceId = message.SystemProperties["iothub-connection-device-id"];
            log.LogInformation($"C# IoT Hub trigger function processed a message: {payload} from {deviceId}");
            UnicodeEncoding uniencoding = new UnicodeEncoding(); 
            byte[] output = uniencoding.GetBytes(payload);
            await outputFile.WriteAsync(output, 0, output.Length);
            var result = JsonSerializer.Deserialize<KineisRoot>(payload);
            foreach (var data in result.Data)
            {
                var parsedData = ParseKineisData(data.RawData);
                log.LogInformation($"Received raw data {data.RawData} which converted to {parsedData.Raw}, Id: {parsedData.Id}, Temperature: {parsedData.Temperature}, IsValid: {parsedData.IsValid}");
                if (parsedData.IsValid) {
                    outputTable.Add(new TelemetryOutput { PartitionKey = "Temperature", RowKey = parsedData.Id.ToString(), Message = parsedData.Temperature.ToString() });
                }
            }
        }

        internal static TelemetryResult ParseKineisData(string data)
        {
            List<string> hexValues = new List<string>();
            // Convert to pairs of hex from 18AFEA to 18 AF EA etc
            for (int i = 0; i < data.Length; i=i+2)
            {
                hexValues.Add(data.Substring(i, 2));
            }
            // Convert to numbers from 18 AF EA to 24 175 234
            List<byte> intValues = new List<byte>();
            foreach (var hexValue in hexValues)
            {
                intValues.Add(byte.Parse(hexValue,System.Globalization.NumberStyles.HexNumber));
            }
            // Convert to chars from 24 175 234 to ascii characters
            var convertedString = string.Join("", intValues.Select(a => (char) a));

            // TODO: Deal with rubbish data coming through
            // Extra user data in format |45|14.6C (ID = 45, Temperature = 14.6C
            var match = Regex.Match(convertedString, @"\|([0-9]{1,3})\|([0-9.]{1,5})C");
            var result = new TelemetryResult
            {
                Raw = convertedString
            };
            // TODO: Extract day and time:
            // Skip 23 bits
            //DAT_DAY_1 5 bits = [1-31] days
            //DAT_HOUR_1 5 bits = [0-23] hours
            //DAT_MIN_1 6 bits = [0-59] minutes

            if (match.Groups.Count == 3)
            {
                result.Id = int.Parse(match.Groups[1].Value);
                result.Temperature = double.Parse(match.Groups[2].Value);
            }
            return result;
        }
    }
    
}