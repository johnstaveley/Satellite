using System;
using System.Text.Json.Serialization;

namespace Receive.Models
{
    public class KineisDatum
    {
        [JsonPropertyName("DEVICE_ID")]
        public string DeviceId { get; set; }
        [JsonPropertyName("MSG_DATE")]
        public DateTime MessageDate { get; set; }
        [JsonPropertyName("MSG_ID")]
        public int MessageId { get; set; }
        [JsonPropertyName("RAW_DATA")]
        public string RawData { get; set; }
    }
}
