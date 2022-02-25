using NUnit.Framework;
using Receive.Models;
using System;
using System.Text.Json;

namespace Receive.Tests
{
    public class DeserialisationTests
    {

        [Test]
        public void Given_Payload_When_Deserialise_Then_ObjectCreatedCorrectly()
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
            Assert.That(firstDatum.MessageDate, Is.EqualTo(DateTime.Parse("2022-02-09 10:21:05.197")));
            Assert.That(firstDatum.RawData, Is.EqualTo("005DE952A37EE80186A0387C37397C31302E33324300376461746366030185"));
            Assert.That(firstDatum.MessageId, Is.EqualTo(401154672));
        }
    }
}