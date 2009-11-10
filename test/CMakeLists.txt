
#build the test executable
INCLUDE_DIRECTORIES( ${GLIB2_INCLUDE_DIR} )

SET(plist_test_SRC
	plist_test.c)
SET(plist_cmp_SRC
	plist_cmp.c)
ADD_EXECUTABLE(plist_test ${plist_test_SRC})
TARGET_LINK_LIBRARIES(plist_test plist)
ADD_EXECUTABLE(plist_cmp ${plist_cmp_SRC})
TARGET_LINK_LIBRARIES(plist_cmp plist)

INCLUDE(CTest)

ADD_TEST(Empty plist_test data/1.plist)
ADD_TEST(Small plist_test data/2.plist)
ADD_TEST(Medium plist_test data/3.plist)
ADD_TEST(Large plist_test data/4.plist)
ADD_TEST(Huge plist_test data/5.plist)
ADD_TEST(Big_Array plist_test data/6.plist)

ADD_TEST(EmptyCmp plist_cmp data/1.plist data/1.plist.out)
ADD_TEST(SmallCmp plist_cmp data/2.plist data/2.plist.out)
ADD_TEST(MediumCmp plist_cmp data/3.plist data/3.plist.out)
ADD_TEST(LargeCmp plist_cmp data/4.plist data/4.plist.out)
ADD_TEST(HugeCmp plist_cmp data/5.plist data/5.plist.out)
ADD_TEST(Big_ArrayCmp plist_cmp data/6.plist data/6.plist.out)
