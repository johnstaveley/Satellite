namespace Receive.Models
{
    public class TelemetryResult
    {
        public string Raw { get; set; }
        public int Id { get; set; }
        public double Temperature { get; set; }
        public bool IsValid => Id != 0 && Temperature > 0; 
    }
}
