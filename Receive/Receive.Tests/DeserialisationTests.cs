using NUnit.Framework;
using Receive.Models;
using System;
using System.Text.Json;

namespace Receive.Tests
{
    public class DeserialisationTests
    {

        [Test]
        public void Given_PayloadWithBasicSchma_When_Deserialise_Then_ObjectCreatedCorrectly()
        {
            // Arrange
            var receivedMessage = "{\"TYPE\":\"DEVICE_RAW\",\"MODE\":\"BASIC\",\"VERSION\":1,\"DATA\":[{\"DEVICE_ID\":\"205895\",\"MSG_DATE\":\"2022-02-09T10:21:05.197Z\",\"MSG_ID\":401154672,\"RAW_DATA\":\"005DE952A37EE80186A0387C37397C31302E33324300376461746366030185\"}]}";

            // Act
            var result = JsonSerializer.Deserialize<KineisRoot>(receivedMessage);

            // Assert
            Assert.That(result.Mode, Is.EqualTo("BASIC"));
            Assert.That(result.Type, Is.EqualTo("DEVICE_RAW"));
            Assert.That(result.Version, Is.EqualTo(1));
            Assert.That(result.Data.Count, Is.EqualTo(1));
            var firstDatum = result.Data[0];
            Assert.That(firstDatum.DeviceId, Is.EqualTo("205895"));
            Assert.That(firstDatum.IsChecked, Is.Null);
            Assert.That(firstDatum.MessageDate, Is.EqualTo(DateTime.Parse("2022-02-09 10:21:05.197")));
            Assert.That(firstDatum.RawData, Is.EqualTo("005DE952A37EE80186A0387C37397C31302E33324300376461746366030185"));
            Assert.That(firstDatum.MessageId, Is.EqualTo(401154672));
        }

        [Test]
        public void Given_PayloadWithExpertSchema_When_Deserialise_Then_ObjectCreatedCorrectly()
        {
            // Arrange
            var receivedMessage = "{\"TYPE\":\"DEVICE_PRC\",\"MODE\":\"EXPERT\",\"VERSION\":1,\"DATA\":[{\"DEVICE_ID\":\"205895\",\"MSG_ID\":2124107,\"CHECKED\":false,\"SENSORS\":{\"GPS_DATE\":\"2022-02-28T10:17:52.122Z\",\"CRC_OK\":false,\"BCH_STATUS\":-1,\"RAW_DATA\":\"FC51D88CCE41210E6D69ABFF88A5C164198F8CF17F0E753090\"}}]}";

            // Act
            var result = JsonSerializer.Deserialize<KineisRoot>(receivedMessage);

            // Assert
            Assert.That(result.Mode, Is.EqualTo("EXPERT"));
            Assert.That(result.Type, Is.EqualTo("DEVICE_PRC"));
            Assert.That(result.Version, Is.EqualTo(1));
            Assert.That(result.Data.Count, Is.EqualTo(1));
            var firstDatum = result.Data[0];
            Assert.That(firstDatum.DeviceId, Is.EqualTo("205895"));
            Assert.That(firstDatum.MessageId, Is.EqualTo(2124107));
            Assert.That(firstDatum.MessageDate, Is.Null);
            Assert.That(firstDatum.IsChecked, Is.False);
            Assert.That(firstDatum.Sensors, Is.Not.Null); 
            Assert.That(firstDatum.Sensors.IsCrcOk, Is.False);
            Assert.That(firstDatum.Sensors.BchStatus, Is.EqualTo(-1));
            Assert.That(firstDatum.Sensors.GpsDate, Is.EqualTo(DateTime.Parse("2022-02-28T10:17:52.122Z")));
            Assert.That(firstDatum.Sensors.RawData, Is.EqualTo("FC51D88CCE41210E6D69ABFF88A5C164198F8CF17F0E753090"));
        }

    }
}