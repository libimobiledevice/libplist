
#build the test executable

SET(plist_test_SRC
	plist_test.c)
ADD_EXECUTABLE(plist_test ${plist_test_SRC})
TARGET_LINK_LIBRARIES(plist_test plist)


INCLUDE(CTest)

ADD_TEST(Empty plist_test data/1.plist)
ADD_TEST(Small plist_test data/2.plist)
ADD_TEST(Medium plist_test data/3.plist)
ADD_TEST(Large plist_test data/4.plist)
ADD_TEST(Huge plist_test data/5.plist)
ADD_TEST(Big_Array plist_test data/6.plist)

