using Microsoft.Azure.EventHubs;
using Microsoft.Azure.WebJobs;
using Microsoft.Extensions.Logging;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using IoTHubTrigger = Microsoft.Azure.WebJobs.EventHubTriggerAttribute;

namespace Receive
{
    public static class IoTHubData
    {

        [FunctionName("IoTHubData")]
        public static async Task Run(
            [IoTHubTrigger("messages/events", Connection = "AzureIoTHubConnectionString", ConsumerGroup = "$Default")] EventData message, 
            [Blob("outputdemo/{sys.utcnow}.txt", FileAccess.Write, Connection = "AzureWebJobsStorage")] Stream outputFile,
            ILogger log)
        {
            var payload = Encoding.UTF8.GetString(message.Body.Array);
            var deviceId = message.SystemProperties["iothub-connection-device-id"];
            log.LogInformation($"C# IoT Hub trigger function processed a message: {payload} from {deviceId}");
            UnicodeEncoding uniencoding = new UnicodeEncoding();
            byte[] output = uniencoding.GetBytes(payload);
            // TODO: Process Kineis payload into something intelligable
            await outputFile.WriteAsync(output, 0, output.Length);
        }
    }
}