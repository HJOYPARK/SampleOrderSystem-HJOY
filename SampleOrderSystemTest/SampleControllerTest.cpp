#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Repository/IRepository.h"
#include "Model/Sample.h"
#include "Controller/SampleController.h"

using ::testing::Return;
using ::testing::_;

class MockSampleRepository : public IRepository<Sample> {
public:
    MOCK_METHOD(void, save, (const Sample&), (override));
    MOCK_METHOD(std::optional<Sample>, findById, (const std::string&), (override));
    MOCK_METHOD(std::vector<Sample>, findAll, (), (override));
    MOCK_METHOD(void, update, (const Sample&), (override));
    MOCK_METHOD(void, remove, (const std::string&), (override));
};

// Step 5-2: Sample registration
TEST(SampleControllerTest, RegisterSampleCallsRepositorySave)
{
    MockSampleRepository mockRepo;
    SampleController controller(&mockRepo);
    EXPECT_CALL(mockRepo, findById("S-001")).WillOnce(Return(std::nullopt));
    EXPECT_CALL(mockRepo, save(_)).Times(1);
    bool result = controller.registerSample("S-001", "Silicon Wafer", 0.5, 0.92, 480);
    EXPECT_TRUE(result);
}

// Step 5-3: Reject duplicate sample ID
TEST(SampleControllerTest, RejectsDuplicateSampleId)
{
    MockSampleRepository mockRepo;
    SampleController controller(&mockRepo);
    Sample existing("S-001", "Existing", 0.5, 0.9, 100);
    EXPECT_CALL(mockRepo, findById("S-001")).WillOnce(Return(existing));
    EXPECT_CALL(mockRepo, save(_)).Times(0);
    bool result = controller.registerSample("S-001", "New", 0.3, 0.8, 0);
    EXPECT_FALSE(result);
}

// Step 5-4: List samples
TEST(SampleControllerTest, ListSamplesReturnsAllFromRepository)
{
    MockSampleRepository mockRepo;
    SampleController controller(&mockRepo);
    std::vector<Sample> samples = {
        Sample("S-001", "Sample A", 0.5, 0.92, 100),
        Sample("S-002", "Sample B", 0.3, 0.78, 50)
    };
    EXPECT_CALL(mockRepo, findAll()).WillOnce(Return(samples));
    auto result = controller.listSamples();
    EXPECT_EQ(result.size(), 2u);
}

// Step 5-5: Search by name
TEST(SampleControllerTest, SearchSamplesByNameReturnsMatching)
{
    MockSampleRepository mockRepo;
    SampleController controller(&mockRepo);
    std::vector<Sample> all = {
        Sample("S-001", "Silicon Wafer", 0.5, 0.92, 100),
        Sample("S-002", "GaN Epitaxial", 0.3, 0.78, 50)
    };
    EXPECT_CALL(mockRepo, findAll()).WillOnce(Return(all));
    auto result = controller.searchByName("Silicon");
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].id, "S-001");
}
