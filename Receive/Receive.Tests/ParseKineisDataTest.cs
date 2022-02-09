using NUnit.Framework;

namespace Receive.Tests
{
    [TestFixture]
    public class ParseKineisDataTest
    {

        [Test]
        public void Given_KineisData_When_Parse_Then_ReturnsConvertedString()
        {
            // Given
            var stringToParse = "005DE952A37EE80186A0387C37397C31302E33324300376461746366030185";

            // Act
            var result = IoTHubData.ParseKineisData(stringToParse);

            // Assert
            Assert.That(result.Contains("8|79|10.32C"));
        }


    }
}
