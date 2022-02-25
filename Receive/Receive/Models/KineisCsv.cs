using CsvHelper.Configuration.Attributes;
using System;

namespace Receive.Models
{
    public class KineisCsv
    {
        [Name("DEVICE_ID")]
        public string DeviceId { get; set; }

        [Name("MSG_ID")]
        public string MessageId { get; set; }

        [Name("CHECKED")]
        public bool Checked { get; set; }

        [Name("GPS_DATE")]
        public DateTime GpsDate { get; set; }

        [Name("CRC_OK")]
        public bool CrcOk { get; set; }

        [Name("BCH_STATUS")]
        public string BchStatus { get; set; }

        [Name("LONG")]

        public string Longitude { get; set; }

        [Name("LAT")]
        public string Latitude { get; set; }

        [Name("ALT")]
        public string Altitude { get; set; }

        [Name("SENSORS")]
        public string Sensors { get; set; }

        [Name("METADATAS")]
        public string Metadatas { get; set; }

        [Name("COUNTER")]
        public string Counter { get; set; }

        [Ignore]
        public string RawSensorData { get; set; }
    }
}
