#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cryfs/impl/config/CryConfigCreator.h>
#include <cryfs/impl/config/CryCipher.h>
#include <cpp-utils/crypto/symmetric/ciphers.h>
#include "../testutils/MockConsole.h"

using namespace cryfs;

using boost::optional;
using boost::none;
using cpputils::Console;
using cpputils::unique_ref;
using cpputils::make_unique_ref;
using std::string;
using std::vector;
using std::shared_ptr;
using std::make_shared;
using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::ValuesIn;
using ::testing::HasSubstr;
using ::testing::UnorderedElementsAreArray;
using ::testing::WithParamInterface;

#define EXPECT_ASK_TO_USE_DEFAULT_SETTINGS()                                                                           \
  EXPECT_CALL(*console, askYesNo("Use default settings?")).Times(1)
#define EXPECT_DOES_NOT_ASK_TO_USE_DEFAULT_SETTINGS()                                                                  \
  EXPECT_CALL(*console, askYesNo("Use default settings?")).Times(0)
#define EXPECT_ASK_FOR_CIPHER()                                                                                        \
  EXPECT_CALL(*console, ask(HasSubstr("block cipher"), UnorderedElementsAreArray(CryCiphers::supportedCipherNames()))).Times(1)
#define EXPECT_DOES_NOT_ASK_FOR_CIPHER()                                                                               \
  EXPECT_CALL(*console, ask(HasSubstr("block cipher"), _)).Times(0)
#define EXPECT_ASK_FOR_BLOCKSIZE()                                                                                     \
  EXPECT_CALL(*console, ask(HasSubstr("block size"), _)).Times(1)
#define EXPECT_DOES_NOT_ASK_FOR_BLOCKSIZE()                                                                            \
  EXPECT_CALL(*console, ask(HasSubstr("block size"), _)).Times(0)

class CryConfigCreatorTest: public ::testing::Test {
public:
    CryConfigCreatorTest()
            : console(make_shared<MockConsole>()),
              creator(console, cpputils::Random::PseudoRandom(), false),
              noninteractiveCreator(console, cpputils::Random::PseudoRandom(), true) {
        EXPECT_CALL(*console, ask(HasSubstr("block cipher"), _)).WillRepeatedly(ChooseAnyCipher());
        EXPECT_CALL(*console, ask(HasSubstr("block size"), _)).WillRepeatedly(Return(0));
    }
    shared_ptr<MockConsole> console;
    CryConfigCreator creator;
    CryConfigCreator noninteractiveCreator;

    void AnswerNoToDefaultSettings() {
        EXPECT_ASK_TO_USE_DEFAULT_SETTINGS().WillOnce(Return(false));
    }

    void AnswerYesToDefaultSettings() {
        EXPECT_ASK_TO_USE_DEFAULT_SETTINGS().WillOnce(Return(true));
    }
};

TEST_F(CryConfigCreatorTest, DoesAskForCipherIfNotSpecified) {
    AnswerNoToDefaultSettings();
    EXPECT_ASK_FOR_CIPHER().WillOnce(ChooseAnyCipher());
    CryConfig config = creator.create(none);
}

TEST_F(CryConfigCreatorTest, DoesNotAskForCipherIfSpecified) {
    AnswerNoToDefaultSettings();
    EXPECT_DOES_NOT_ASK_FOR_CIPHER();
    CryConfig config = creator.create(string("aes-256-gcm"));
}

TEST_F(CryConfigCreatorTest, DoesNotAskForCipherIfUsingDefaultSettings) {
    AnswerYesToDefaultSettings();
    EXPECT_DOES_NOT_ASK_FOR_CIPHER();
    CryConfig config = creator.create(none);
}

TEST_F(CryConfigCreatorTest, DoesNotAskForCipherIfNoninteractive) {
    EXPECT_DOES_NOT_ASK_TO_USE_DEFAULT_SETTINGS();
    EXPECT_DOES_NOT_ASK_FOR_CIPHER();
    CryConfig config = noninteractiveCreator.create(none);
}

TEST_F(CryConfigCreatorTest, DoesAskForBlocksizeIfNotSpecified) {
    AnswerNoToDefaultSettings();
    EXPECT_ASK_FOR_BLOCKSIZE().WillOnce(Return(1));
    CryConfig config = creator.create(none);
}

//TODO DoesNotAskForCipherIfSpecified

TEST_F(CryConfigCreatorTest, DoesNotAskForBlocksizeIfNoninteractive) {
    EXPECT_DOES_NOT_ASK_TO_USE_DEFAULT_SETTINGS();
    EXPECT_DOES_NOT_ASK_FOR_BLOCKSIZE();
    CryConfig config = noninteractiveCreator.create(none);
}

TEST_F(CryConfigCreatorTest, DoesNotAskForBlocksizeIfUsingDefaultSettings) {
    AnswerYesToDefaultSettings();
    EXPECT_DOES_NOT_ASK_FOR_BLOCKSIZE();
    CryConfig config = creator.create(none);
}

TEST_F(CryConfigCreatorTest, ChoosesEmptyRootBlobId) {
    AnswerNoToDefaultSettings();
    CryConfig config = creator.create(none);
    EXPECT_EQ("", config.RootBlob()); // This tells CryFS to create a new root blob
}

TEST_F(CryConfigCreatorTest, ChoosesValidEncryptionKey_448) {
    AnswerNoToDefaultSettings();
    EXPECT_ASK_FOR_CIPHER().WillOnce(ChooseCipher("mars-448-gcm"));
    CryConfig config = creator.create(none);
    cpputils::Mars448_GCM::EncryptionKey::FromString(config.EncryptionKey()); // This crashes if invalid
}

TEST_F(CryConfigCreatorTest, ChoosesValidEncryptionKey_256) {
    AnswerNoToDefaultSettings();
    EXPECT_ASK_FOR_CIPHER().WillOnce(ChooseCipher("aes-256-gcm"));
    CryConfig config = creator.create(none);
    cpputils::AES256_GCM::EncryptionKey::FromString(config.EncryptionKey()); // This crashes if invalid
}

TEST_F(CryConfigCreatorTest, ChoosesValidEncryptionKey_128) {
    AnswerNoToDefaultSettings();
    EXPECT_ASK_FOR_CIPHER().WillOnce(ChooseCipher("aes-128-gcm"));
    CryConfig config = creator.create(none);
    cpputils::AES128_GCM::EncryptionKey::FromString(config.EncryptionKey()); // This crashes if invalid
}

//TODO Add test cases ensuring that the values entered are correctly taken