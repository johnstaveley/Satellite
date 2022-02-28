using System;
using System.Text.Json.Serialization;

namespace Receive.Models
{
    public class KineisSensors
    {
        [JsonPropertyName("GPS_DATE")]
        public DateTime? GpsDate { get; set; }
        [JsonPropertyName("BCH_STATUS")]
        public int? BchStatus { get; set; }
        [JsonPropertyName("RAW_DATA")]
        public string RawData { get; set; }
        [JsonPropertyName("CRC_OK")]
        public bool? IsCrcOk { get; set; }
    }
}
