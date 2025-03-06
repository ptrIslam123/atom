#include <gtest/gtest.h>

#include "include/memory/allocators/shared_object_pool.h"

#include <string>
#include <unordered_set>
#include <vector>
#include <random>

// namespace {

// struct XObject {
//     XObject(std::string _id): id(_id), data(_id + " data") {}
//     std::string id;
//     std::string data;
// };

// std::string GenerateRandomUUIDString(size_t length) {
//     const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
//     std::string result;
//     result.reserve(length);

//     std::random_device rd; // Uses hardware entropy if available
//     std::uniform_int_distribution<> distribution(0, charset.size() - 1);

//     for (size_t i = 0; i < length; ++i) {
//         result += charset[distribution(rd)];
//     }

//     return result;
// }

// bool operator==(const XObject& f, const XObject& s) {
//     return (f.id == s.id) && (f.data == s.data);
// }

// struct XObjectHash {
//     std::size_t operator()(const XObject& object) const noexcept {
//         return std::hash<std::string>{}(object.id);
//     }
// };

// struct XObjectEq {
//     bool operator()(const XObject& f, const XObject& s) const {
//         return f == s;
//     }
// };

// } //! namespace

// using namespace atom::memory::allocator;

// TEST(TestMemPool, Test1) {
//     NonThreadSafeSharedObjectPool<XObject, XObjectHash, XObjectEq> pool;

//     XObject object{GenerateRandomUUIDString(8)};
//     auto ptr1 = pool.allocate<XObject>(object.id);
//     auto ptr2 = pool.allocate<XObject>(object.id);
//     EXPECT_EQ(*ptr1, *ptr2);
// }

// TEST(TestMemPool, Test2) {
//     std::vector<std::string> idSet;
//     std::unordered_set<XObject, XObjectHash, XObjectEq> objects;
//     NonThreadSafeSharedObjectPool<XObject, XObjectHash, XObjectEq> pool;
//     for (auto i = 0; i < 1000; ++i) {
//         std::string id = GenerateRandomUUIDString(8);
//         XObject object{id};
//         if (objects.contains(id)) {
//             continue;
//         }
//         objects.insert(std::move(object));
//         idSet.push_back(id);
//     }

//     pool.reserve(objects.size());

//     for (const auto& [k, _] : objects) {
//         (void)pool.allocate<XObject>(k);
//     }

//     for (auto i = 0; i < objects.size() * 5; ++i) {
//         auto index = rand() % idSet.size();
//         const auto& id = idSet[index];
//         auto ptr = pool.allocate<XObject>(id);
//         auto it = objects.find(id);
//         ASSERT_NE(it, objects.cend());
//         const auto& object = *it;
//         ASSERT_EQ(*ptr, object);
//     }

//     for (const auto& id : idSet) {
//         auto ptr = pool.allocate<XObject>(id);
//         pool.deallocate(std::move(ptr));
//     }
// }

// TEST(TestMemPool, TestWithPolymorphicTypes) {
//     struct Base {
//         std::string m_base;
//         Base(std::string base): m_base(base) {}

//         virtual ~Base() = default;
//     };

//     struct Derived : Base {
//         Derived(std::string base, std::string derived): Base(base), m_derived(derived) {}
//         std::string m_derived;
//     };

//     struct Hash {
//         std::size_t operator()(const Base& base) const noexcept {
//             return std::hash<std::string>{}.operator()(base.m_base);
//         };
//     };

//     struct Eq {
//         bool operator()(const Base& f, const Base& s) const noexcept {
//             return f.m_base == s.m_base;
//         }
//     };

//     NonThreadSafeSharedObjectPool<Base, Hash, Eq> pool;

//     (void)pool.allocate<Derived>("b1", "d1");
//     (void)pool.allocate<Derived>("b2", "d2");

//     auto ptr1 = pool.allocate<Derived>("b1", "d1");
//     ASSERT_NE(ptr1, nullptr);
//     EXPECT_EQ(ptr1->m_base, "b1");

//     // pool.allocate<std::string>("dfdfdf"); /* compile error! */
// }
