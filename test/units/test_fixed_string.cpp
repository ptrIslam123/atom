#include <gtest/gtest.h>

#include "include/containers/fixed/string.h"
#include "include/iterator/iterator.h"

using namespace atom::containers::fixed;
using namespace atom::iter;

// TEST(FixedString, Constructors) {
//     // Default constructor
//     String<10> empty;
//     EXPECT_TRUE(empty.isEmpty());
//     EXPECT_EQ(empty.size(), 0);

//     // From C-string
//     String<10> from_cstr("hello");
//     EXPECT_EQ(from_cstr.size(), 5);
//     EXPECT_TRUE(from_cstr.compare("hello"));

//     // From character
//     String<10> from_char('A');
//     EXPECT_EQ(from_char.size(), 1);
//     EXPECT_EQ(from_char[0], 'A');

//     // From character count
//     String<10> from_char_count('X', String<10>::SizeType{3});
//     EXPECT_EQ(from_char_count.size(), 3);
//     EXPECT_TRUE(from_char_count.compare("XXX"));

//     // From initializer list
//     String<10> from_ilist{'h', 'e', 'l', 'l', 'o'};
//     EXPECT_EQ(from_ilist.size(), 5);
//     EXPECT_TRUE(from_ilist.compare("hello"));

//     // Copy constructor
//     String<10> copy = from_cstr;
//     EXPECT_TRUE(copy.compare("hello"));

//     // Move constructor
//     String<10> moved = std::move(from_cstr);
//     EXPECT_TRUE(moved.compare("hello"));
//     EXPECT_TRUE(from_cstr.isEmpty()); // Should be empty after move
// }

// TEST(FixedString, AssignmentOperators) {
//     String<10> str;

//     // From C-string
//     str = "hello";
//     EXPECT_TRUE(str.compare("hello"));

//     // From character
//     str = 'A';
//     EXPECT_TRUE(str.compare("A"));

//     // From initializer list
//     str = {'t', 'e', 's', 't'};
//     EXPECT_TRUE(str.compare("test"));

//     // Copy assignment
//     String<10> other = "world";
//     str = other;
//     EXPECT_TRUE(str.compare("world"));

//     // Move assignment
//     String<10> source = "source";
//     str = std::move(source);
//     EXPECT_TRUE(str.compare("source"));
//     EXPECT_TRUE(source.isEmpty());
// }

// TEST(FixedString, AppendOperations) {
//     String<20> str;

//     // Append character
//     str.append('H');
//     EXPECT_EQ(str.size(), 1);
//     EXPECT_EQ(str[0], 'H');

//     // Append C-string
//     str.append("ello");
//     EXPECT_TRUE(str.compare("Hello"));

//     // Append with count
//     str.append(" World", 6);
//     EXPECT_TRUE(str.compare("Hello World"));

//     // Append another string
//     String<20> other("!!!");
//     str.append(other);
//     EXPECT_TRUE(str.compare("Hello World!!!"));

//     // Append initializer list
//     str.append({' ', ':', ')'});
//     EXPECT_TRUE(str.compare("Hello World!!! :)"));

//     // Test operator+=
//     str += String<20>{" test"};
//     EXPECT_TRUE(str.compare("Hello World!!! :) test"));
// }

// TEST(FixedString, InsertOperations) {
//     String<20> str("Hello World");

//     // Insert character in middle
//     auto it = str.insert(str.firstConstIter() + 5, '!');
//     EXPECT_TRUE(str.compare("Hello! World"));
//     EXPECT_EQ(*it, '!');

//     // Insert C-string at beginning
//     it = str.insert(str.firstConstIter(), "Say ");
//     EXPECT_TRUE(str.compare("Say Hello! World"));
//     EXPECT_EQ(*it, 'S');

//     // Insert at end
//     it = str.insert(str.endConstIter(), "!");
//     EXPECT_TRUE(str.compare("Say Hello! World!"));
//     EXPECT_EQ(it, str.endConstIter() - 1);

//     // Insert with count
//     it = str.insert(str.firstConstIter() + 4, "***", 2);
//     EXPECT_TRUE(str.compare("Say **Hello! World!"));

//     // Insert another string
//     String<20> insert_str("Beautiful ");
//     it = str.insert(str.firstConstIter() + 8, insert_str);
//     EXPECT_TRUE(str.compare("Say **Beautiful Hello! World!"));
// }

// TEST(FixedString, EraseOperations) {
//     String<20> str("Hello World");

//     // Erase single character
//     auto it = str.erase(str.firstConstIter() + 5);
//     EXPECT_TRUE(str.compare("HelloWorld"));
//     EXPECT_EQ(*it, 'W');

//     // Erase range
//     it = str.erase(str.firstConstIter() + 5, str.firstConstIter() + 10);
//     EXPECT_TRUE(str.compare("Hello"));
//     EXPECT_EQ(it, str.endConstIter());

//     // Erase from beginning
//     it = str.erase(str.firstConstIter(), str.firstConstIter() + 2);
//     EXPECT_TRUE(str.compare("llo"));
//     EXPECT_EQ(*it, 'l');

//     // Erase everything
//     it = str.erase(str.firstConstIter(), str.endConstIter());
//     EXPECT_TRUE(str.isEmpty());
//     EXPECT_EQ(it, str.endConstIter());
// }

// TEST(FixedString, AccessOperations) {
//     String<10> str("Hello");

//     // Operator[]
//     EXPECT_EQ(str[0], 'H');
//     EXPECT_EQ(str[4], 'o');

//     // atUnsafe
//     EXPECT_EQ(str.atUnsafe(1), 'e');
//     EXPECT_EQ(str.atUnsafe(3), 'l');

//     // first/last element
//     EXPECT_EQ(str.firstElement(), 'H');
//     EXPECT_EQ(str.lastElement(), 'o');

//     // data() should be null-terminated
//     EXPECT_STREQ(str.data(), "Hello");
// }

// TEST(FixedString, Iterators) {
//     String<10> str("Hello");

//     // Forward iteration
//     auto it = str.firstConstIter();
//     EXPECT_EQ(*it++, 'H');
//     EXPECT_EQ(*it++, 'e');
//     EXPECT_EQ(*it++, 'l');
//     EXPECT_EQ(*it++, 'l');
//     EXPECT_EQ(*it++, 'o');
//     EXPECT_EQ(it, str.endConstIter());

//     // Reverse iteration
//     auto rit = str.firstReverseConstIter();
//     EXPECT_EQ(*rit++, 'o');
//     EXPECT_EQ(*rit++, 'l');
//     EXPECT_EQ(*rit++, 'l');
//     EXPECT_EQ(*rit++, 'e');
//     EXPECT_EQ(*rit++, 'H');
//     EXPECT_EQ(rit, str.endReverseConstIter());
// }

// TEST(FixedString, CapacityOperations) {
//     String<5> str;

//     EXPECT_TRUE(str.isEmpty());
//     EXPECT_FALSE(str.isFull());
//     EXPECT_EQ(str.capacity(), 5);

//     str = "test";
//     EXPECT_FALSE(str.isEmpty());
//     EXPECT_FALSE(str.isFull());

//     str.pushBack('!');
//     EXPECT_TRUE(str.isFull());
//     EXPECT_EQ(str.size(), 5);

//     // Try to overflow
//     str.pushBack('X'); // Should not crash but shouldn't add
//     EXPECT_EQ(str.size(), 5);
//     EXPECT_TRUE(str.isFull());
// }

// TEST(FixedString, ComparisonOperations) {
//     String<10> str1("hello");
//     String<10> str2("hello");
//     String<10> str3("world");

//     // Operator==
//     EXPECT_TRUE(str1 == str2);
//     EXPECT_FALSE(str1 == str3);

//     // compare method
//     EXPECT_TRUE(str1.compare(str2));
//     EXPECT_FALSE(str1.compare(str3));

//     // Different sizes
//     String<10> str4("hell");
//     EXPECT_FALSE(str1 == str4);
// }

// TEST(FixedString, SubStringOperations) {
//     String<10> str("HelloWorld");

//     // Full substring
//     auto sub1 = str.subString();
//     EXPECT_TRUE(sub1.compare("HelloWorld"));

//     // Partial substring
//     auto sub2 = str.subString(0, 5);
//     EXPECT_TRUE(sub2.compare("Hello"));

//     // Middle substring
//     auto sub3 = str.subString(5, 5);
//     EXPECT_TRUE(sub3.compare("World"));

//     // With npos
//     auto sub4 = str.subString(3);
//     EXPECT_TRUE(sub4.compare("loWorld"));

//     // Copy
//     auto copy = str.copy();
//     EXPECT_TRUE(copy.compare("HelloWorld"));
// }

// TEST(FixedString, FindOperations) {
//     String<20> str("Hello World Hello");

//     // Find character
//     auto it = str.find('W');
//     EXPECT_NE(it, str.endConstIter());
//     EXPECT_EQ(*it, 'W');

//     // Find non-existent character
//     it = str.find('X');
//     EXPECT_EQ(it, str.endConstIter());

//     // Find C-string
//     it = str.find("World");
//     EXPECT_NE(it, str.endConstIter());
//     EXPECT_EQ(*it, 'W');

//     // Find with position
//     it = str.find('l', 5);
//     EXPECT_EQ(Distance(str.firstIter(), it), 9);

//     // Find from another string
//     String<20> pattern("llo");
//     it = str.find(pattern);
//     EXPECT_NE(it, str.endConstIter());
//     EXPECT_EQ(*it, 'l');
// }

// TEST(FixedString, ContainsOperations) {
//     String<20> str("Hello World");

//     // Contains character
//     EXPECT_TRUE(str.contains('H'));
//     EXPECT_TRUE(str.contains(' '));
//     EXPECT_FALSE(str.contains('X'));

//     // Contains C-string
//     EXPECT_TRUE(str.contains("Hello"));
//     EXPECT_TRUE(str.contains("World"));
//     EXPECT_FALSE(str.contains("Universe"));

//     // Contains with count
//     EXPECT_TRUE(str.contains("Hello", 5));
//     EXPECT_TRUE(str.contains("World", 5));

//     // Contains from another string
//     String<20> pattern("lo W");
//     EXPECT_TRUE(str.contains(pattern));
// }

// TEST(FixedString, PushPopOperations) {
//     String<10> str;

//     // Push back
//     str.pushBack('H');
//     str.pushBack('e');
//     str.pushBack('l');
//     str.pushBack('l');
//     str.pushBack('o');
//     EXPECT_TRUE(str.compare("Hello"));

//     // Pop back
//     str.popBack();
//     EXPECT_TRUE(str.compare("Hell"));
//     str.popBack();
//     EXPECT_TRUE(str.compare("Hel"));

//     // Clear
//     str.clear();
//     EXPECT_TRUE(str.isEmpty());
// }

// TEST(FixedString, OverflowProtection) {
//     String<5> str;

//     // Try to insert beyond capacity
//     str = "hello"; // Exactly 5 characters
//     EXPECT_TRUE(str.isFull());

//     // These should not crash but should not modify string
//     str.pushBack('!');
//     EXPECT_TRUE(str.compare("hello"));

//     str.append("!!!");
//     EXPECT_TRUE(str.compare("hello"));

//     str.insert(str.endConstIter(), "extra");
//     EXPECT_TRUE(str.compare("hello"));
// }

// TEST(FixedString, MoveSemantics) {
//     String<10> source("Hello");
//     EXPECT_EQ(source.size(), 5);

//     // Move construction
//     String<10> dest(std::move(source));
//     EXPECT_TRUE(dest.compare("Hello"));
//     EXPECT_TRUE(source.isEmpty());
//     EXPECT_EQ(source.size(), 0);

//     // Move assignment
//     String<10> other("World");
//     dest = std::move(other);
//     EXPECT_TRUE(dest.compare("World"));
//     EXPECT_TRUE(other.isEmpty());
// }

// TEST(FixedString, VariousCharTypes) {
//     // Test different character types
//     // WString<10> wstr(L"Hello");
//     // EXPECT_EQ(wstr.size(), 5);

//     // U8String<10> u8str(u8"Hello");
//     // EXPECT_EQ(u8str.size(), 5);

//     // U16String<10> u16str(u"Hello");
//     // EXPECT_EQ(u16str.size(), 5);

//     // U32String<10> u32str(U"Hello");
//     // EXPECT_EQ(u32str.size(), 5);
// }

// TEST(FixedString, EdgeCases) {
//     // Empty string operations
//     String<10> empty;
//     EXPECT_TRUE(empty.isEmpty());

//     auto it = empty.erase(empty.endConstIter()); // Should do nothing
//     EXPECT_EQ(it, empty.endConstIter());

//     // Single character
//     String<10> single("A");
//     EXPECT_EQ(single.size(), 1);
//     EXPECT_EQ(single[0], 'A');

//     // Full capacity
//     String<3> full("123");
//     EXPECT_TRUE(full.isFull());
//     EXPECT_EQ(full.size(), 3);
// }

// TEST(FixedString, IteratorValidity) {
//     String<10> str("Hello");

//     auto it = str.firstConstIter() + 2;
//     EXPECT_EQ(*it, 'l');

//     // Insert before iterator - iterator should remain valid
//     str.insert(str.firstConstIter(), "X");
//     EXPECT_EQ(*it, 'l'); // Should still point to 'l'
//     EXPECT_TRUE(str.compare("XHello"));

//     // Erase before iterator
//     it = str.firstConstIter() + 3;
//     str.erase(str.firstConstIter());
//     EXPECT_EQ(*it, 'l');
//     EXPECT_TRUE(str.compare("Hello"));
// }
