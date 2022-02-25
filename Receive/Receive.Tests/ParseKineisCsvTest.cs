using NUnit.Framework;
using System;
using System.IO;
using System.Linq;
using System.Reflection;

namespace Receive.Tests
{
    [TestFixture]
    public class ParseKineisCsvTest
    {

        [Test]
        public void Given_KineisCsv_When_Parse_Then_ReturnsConvertedString()
        {
            // Arrange
            var assembly = Assembly.GetExecutingAssembly();
            var resourceName = "Receive.Tests.TestFiles.File1.txt";
            string input;
            using (Stream stream = assembly.GetManifestResourceStream(resourceName))
            using (StreamReader reader = new StreamReader(stream))
            {
                input = reader.ReadToEnd();
            }

            // Act
            var result = IoTHubData.ParseKineisCsv(input);

            // Assert
            Assert.That(result.Count(), Is.EqualTo(1));
            var firstLine = result.First();
            Assert.That(firstLine.DeviceId, Is.EqualTo("205895"));
            Assert.That(firstLine.Altitude, Is.Empty);
            Assert.That(firstLine.MessageId, Is.EqualTo("2095769"), "Message Id is incorrect");
            Assert.That(firstLine.Checked, Is.True);
            Assert.That(firstLine.GpsDate, Is.EqualTo(DateTime.Parse("2022-02-23T16:48:12.504Z")));
            Assert.That(firstLine.CrcOk, Is.True);
            Assert.That(firstLine.BchStatus, Is.Empty);
            Assert.That(firstLine.Sensors, Is.Not.Empty);
            Assert.That(firstLine.RawSensorData, Is.EqualTo("F76AC36EE80186A0387C31387C32302E323443000000000000"));

        }


    }
}
