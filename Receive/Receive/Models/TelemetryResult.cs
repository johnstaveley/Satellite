namespace Receive.Models
{
    public class TelemetryResult
    {
        public string Converted { get; set; }
        public int Id { get; set; }
        public double Temperature { get; set; }
        public byte Day { get; set; }
        public byte Hour { get; set; }
        public byte Minute { get; set; }
        public bool IsValid => Id != 0 && Temperature > 0;
    }
}
