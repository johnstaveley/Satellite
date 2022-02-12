using NUnit.Framework;
using NUnit.Framework.Constraints;

namespace Receive.Tests
{
    [TestFixture]
    public class ParseKineisDataTest
    {

        [Test]
        [TestCase("005DE952A37EE80186A0387C37397C31302E33324300376461746366030185", "|79|10.32C", 79, 10.32, true)]
        [TestCase("570783878F0F01F0F0F0F2F1E1E18F1E31F1CE8C387DC0A1F03970F8F1C58A", "N/A", 0, 0, false)]
        [TestCase("5ECAEA3C636EE80186A0387C3131317C342E312943000000000000104C5A30", "|111|4.1)C", 0, 0, false)]
        [TestCase("43E5E22DE36EE80186A0387C3930307C352E33364300080000000000CAAB70", "|900|5.36C", 900, 5.36, true)]
        [TestCase("A98EEA23436EE8018620387C39327C352E303743000000000000007E372150", "|92|5.07C", 92, 5.07, true)]
        [TestCase("8AFFEA58436EC80186A0387C3133327C372E39334300000000000073A2A200", "|132|7.93C", 132, 7.93, true)]
        [TestCase("92A8EA5D236EE80186A0387C3133367C392E32334300000000000085DFCE40", "|136|9.23C", 136, 9.23, true)]
        [TestCase("C035EB0B636EE80186A0387C3232307C332E3539430060000000004CBDFA30", "|220|3.59C", 220, 3.59,true)]
        [TestCase("0B91EB25236CE80106A0383C323430779FC3C2E98600000020040090EDCA61", "N/A", 0, 0, false)]
        [TestCase("068BE341836EE80086A0387C3236317C302E39344300080000400016F49320", "|261|0.94C", 261, 0.94, true)]
        public void Given_KineisData_When_Parse_Then_ReturnsConvertedString(string stringToParse, string expectedUserData, int expectedId, double expectedTemperature, bool expectedIsValid)
        {
            // Arrange

            // Act
            var result = IoTHubData.ParseKineisData(stringToParse);

            // Assert
            if (expectedIsValid) {
                Assert.That(result.Converted.Contains(expectedUserData), "Did not contain expected user data");
            }
            Assert.That(result.Id, Is.EqualTo(expectedId));
            Assert.That(result.Temperature, Is.EqualTo(expectedTemperature));
            Assert.That(result.IsValid, Is.EqualTo(expectedIsValid));

        }


    }
}
