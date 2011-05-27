
#build the test executable
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/libcnary/include )

SET(plist_test_SRC
	plist_test.c)
SET(plist_cmp_SRC
	plist_cmp.c)
ADD_EXECUTABLE(plist_test ${plist_test_SRC})
TARGET_LINK_LIBRARIES(plist_test plist)
ADD_EXECUTABLE(plist_cmp ${plist_cmp_SRC})
TARGET_LINK_LIBRARIES(plist_cmp plist)

INCLUDE(CTest)

SET(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR})
ADD_TEST(Empty plist_test ${TEST_DATA_DIR}/data/1.plist)
ADD_TEST(Small plist_test ${TEST_DATA_DIR}/data/2.plist)
ADD_TEST(Medium plist_test ${TEST_DATA_DIR}/data/3.plist)
ADD_TEST(Large plist_test ${TEST_DATA_DIR}/data/4.plist)
ADD_TEST(Huge plist_test ${TEST_DATA_DIR}/data/5.plist)
ADD_TEST(Big_Array plist_test ${TEST_DATA_DIR}/data/6.plist)

ADD_TEST(EmptyCmp plist_cmp ${TEST_DATA_DIR}/data/1.plist ${TEST_DATA_DIR}/data/1.plist.out)
ADD_TEST(SmallCmp plist_cmp ${TEST_DATA_DIR}/data/2.plist ${TEST_DATA_DIR}/data/2.plist.out)
ADD_TEST(MediumCmp plist_cmp ${TEST_DATA_DIR}/data/3.plist ${TEST_DATA_DIR}/data/3.plist.out)
ADD_TEST(LargeCmp plist_cmp ${TEST_DATA_DIR}/data/4.plist ${TEST_DATA_DIR}/data/4.plist.out)
ADD_TEST(HugeCmp plist_cmp ${TEST_DATA_DIR}/data/5.plist ${TEST_DATA_DIR}/data/5.plist.out)
ADD_TEST(Big_ArrayCmp plist_cmp ${TEST_DATA_DIR}/data/6.plist ${TEST_DATA_DIR}/data/6.plist.out)
