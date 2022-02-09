using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace Receive.Models
{
    public class KineisRoot
    {
        [JsonPropertyName("TYPE")]
        public string Type { get; set; }
        [JsonPropertyName("MODE")]
        public string Mode { get; set; }
        [JsonPropertyName("VERSION")]
        public int Version { get; set; }
        [JsonPropertyName("DATA")]
        public List<KeneisDatum> Data { get; set; }
    }
}
