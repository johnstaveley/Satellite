namespace Receive.Models
{
    public class TelemetryOutput
    {
        public string PartitionKey { get; set; }
        public string RowKey { get; set; }
        public string Message { get; set; }
    }

}
