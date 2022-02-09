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

[assembly: InternalsVisibleTo("Receive.Tests")]
namespace Receive
{
    public static class IoTHubData
    {

        [FunctionName("IoTHubData")]
        public static async Task Run(
            [IoTHubTrigger("messages/events", Connection = "AzureIoTHubConnectionString", ConsumerGroup = "$Default")] EventData message, 
            [Blob("kineis/{sys.utcnow}.txt", FileAccess.Write, Connection = "AzureWebJobsStorage")] Stream outputFile,
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
                log.LogInformation($"Received raw data {data} which converted to {parsedData}");
            }
        }

        internal static string ParseKineisData(string data)
        {
            List<string> hexValues = new List<string>();
            // Convert to pairs of hex
            for (int i = 0; i < data.Length; i=i+2)
            {
                hexValues.Add(data.Substring(i, 2));
            }
            // Convert to numbers
            List<int> intValues = new List<int>();
            foreach (var hexValue in hexValues)
            {
                intValues.Add(int.Parse(hexValue,System.Globalization.NumberStyles.HexNumber));
            }
            // Convert to chars
            return string.Join("", intValues.Select(a => (char) a));
        }
    }
    
}