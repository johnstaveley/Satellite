using NUnit.Framework;
using System.Collections.Generic;

namespace Receive.Tests
{
    [TestFixture]
    public class GetBytesFromBitsTests
    {

        [Test]
        [TestCase(0,4)]
        [TestCase(4,3)]
        public void Given_EmptyByte_When_ExtractNumberFromBits_Then_EmptyByteReturned(int startBit, int numberOfBits)
        {
            // Arange

            // Act
            var result = IoTHubData.ExtractNumberFromBits(new List<byte> {new byte()}, startBit, numberOfBits);

            // Assert
            Assert.That(result, Is.Not.Null);
            Assert.That(result, Is.EqualTo(0));
        }

        [Test]
        [TestCase(0,4, 15)]
        [TestCase(4,3, 7)]
        [TestCase(0,0, 0)]
        [TestCase(1,1, 1)]
        [TestCase(0,8, 255)]
        public void Given_SubsetOfFullByte_When_ExtractNumberFromBits_Then_CorrectValueReturned(int startBit, int numberOfBits, int expectedValue)
        {
            // Arange

            // Act
            var result = IoTHubData.ExtractNumberFromBits(new List<byte> {(byte) 255}, startBit, numberOfBits);

            // Assert
            Assert.That(result, Is.Not.Null);
            Assert.That(result, Is.EqualTo(expectedValue));
        }

        [Test]
        [TestCase(0,0, 0)]
        [TestCase(0,1, 1)]
        [TestCase(1,1, 0)]
        [TestCase(2,1, 1)]
        [TestCase(0,3, 5)]
        [TestCase(2,3, 5)]
        [TestCase(4,3, 5)]
        public void Given_SubsetOfOddSetBits_When_ExtractNumberFromBits_Then_CorrectValueReturned(int startBit, int numberOfBits, int expectedValue)
        {
            // Arange
            byte initial = 0b1010101;

            // Act
            var result = IoTHubData.ExtractNumberFromBits(new List<byte> {initial}, startBit, numberOfBits);

            // Assert
            Assert.That(result, Is.Not.Null);
            Assert.That(result, Is.EqualTo(expectedValue));
        }

        [Test]
        [TestCase(0,0, 0)]
        [TestCase(0,1, 0)]
        [TestCase(1,1, 1)]
        [TestCase(2,1, 0)]
        [TestCase(0,3, 2)]
        [TestCase(2,3, 2)]
        [TestCase(4,3, 2)]
        [TestCase(1,5, 21)]
        public void Given_SubsetOfEvenSetBits_When_ExtractNumberFromBits_Then_CorrectValueReturned(int startBit, int numberOfBits, int expectedValue)
        {
            // Arange
            byte initial = 0b0101010;

            // Act
            var result = IoTHubData.ExtractNumberFromBits(new List<byte> {initial}, startBit, numberOfBits);

            // Assert
            Assert.That(result, Is.Not.Null);
            Assert.That(result, Is.EqualTo(expectedValue));
        }

        [Test]
        [TestCase(6,3, 5)]
        [TestCase(6,5, 21)]
        public void Given_MultipleInputs_When_ExtractNumberFromBits_Then_CorrectValueReturned(int startBit, int numberOfBits, int expectedValue)
        {
            // Arange
            byte input1 = 0b01010101;
            byte input2 = 0b01010101;

            // Act
            var result = IoTHubData.ExtractNumberFromBits(new List<byte> {input1, input2}, startBit, numberOfBits);

            // Assert
            Assert.That(result, Is.Not.Null);
            Assert.That(result, Is.EqualTo(expectedValue));
        }

    }
}
