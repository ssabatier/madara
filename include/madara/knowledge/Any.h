#ifndef MADARA_KNOWLEDGE_ANY_H_
#define MADARA_KNOWLEDGE_ANY_H_

/**
 * @file Any.h
 * @author David Kyle <david.kyle@shield.ai>
 *
 * This file contains the Any and ConstAny classes
 **/

#include <memory>
#include <sstream>
#include <map>
#include <functional>
#include <type_traits>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include "madara/utility/SupportTest.h"
#include "madara/MadaraExport.h"
#include "madara/utility/StdInt.h"
#include "madara/logger/GlobalLogger.h"
#include "madara/utility/IntTypes.h"
#include "madara/exceptions/BadAnyAccess.h"
#include "TypeHandlers.h"
#include "ConstAnyRef.h"
#include "AnyRef.h"

namespace madara { namespace knowledge {

/**
 * Class which defines methods common to ConstAny and Any. Use those classes
 * instead of this class directly.
 **/
template<typename Impl, typename BaseImpl>
class BasicOwningAny : public BaseImpl, public AnyRegistry
{
  using Base = BaseImpl;

  template<typename Impl2, typename Base2>
  friend class ::madara::knowledge::BasicOwningAny;
public:
  /**
   * Default constructor. Creates an empty Any.
   **/
  BasicOwningAny() = default;

  /**
   * Copy constructor. Will clone any data stored inside.
   **/
  BasicOwningAny(const BasicOwningAny &other)
    : Base(other.handler_,
        other.data_ ?
        (other.handler_ ?
          other.handler_->clone(other.data_) :
          Base::raw_data_storage::clone(other.data_)) : nullptr) {}

  /**
   * Copy constructor. Will clone any data stored inside.
   **/
  template<typename I, typename B>
  BasicOwningAny(const BasicOwningAny<I, B> &other)
    : Base(other.handler_,
        other.data_ ?
        (other.handler_ ?
          other.handler_->clone(other.data_) :
          Base::raw_data_storage::clone(other.data_)) : nullptr) {}

  /**
   * Construct from a ConstAnyRef. Will clone the data it refers to.
   **/
  BasicOwningAny(const ConstAnyRef &other)
    : Base(other.handler_,
        other.data_ ?
        (other.handler_ ?
          other.handler_->clone(other.data_) :
          Base::raw_data_storage::clone(other.data_)) : nullptr) {}

  /**
   * Construct from a AnyRef. Will clone the data it refers to.
   **/
  BasicOwningAny(const AnyRef &other)
    : BasicOwningAny(ConstAnyRef(other)) {}

  /**
   * Copy assignment operator. Will clone any data stored inside.
   **/
  BasicOwningAny &operator=(const BasicOwningAny &other)
  {
    void *data = other.data_ ?
      (other.handler_ ?
        other.handler_->clone(other.data_) :
        Base::raw_data_storage::clone(other.data_)) : nullptr;

    clear();
    this->handler_ = other.handler_;
    this->data_ = data;
    return *this;
  }

  /**
   * Move constructor. Other Any will be left empty.
   **/
  BasicOwningAny(BasicOwningAny &&other) noexcept :
    Base(take_ptr(other.handler_),
      take_ptr(other.data_)) {}

  /**
   * Move constructor. Other Any will be left empty.
   **/
  template<typename I, typename B>
  BasicOwningAny(BasicOwningAny<I, B> &&other) noexcept :
    Base(take_ptr(other.handler_),
      take_ptr(other.data_)) {}

  /**
   * Move assignment operator. Other Any will be left empty.
   **/
  BasicOwningAny &operator=(BasicOwningAny &&other) noexcept
  {
    if (this != &other) {
      using std::swap;
      swap(this->data_, other.data_);
      swap(this->handler_, other.handler_);
    }
    return *this;
  }

  /**
   * Destructor. Deletes the stored data.
   **/
  ~BasicOwningAny() noexcept
  {
    clear();
  }

  /**
   * Clear and free the data inside this Any. Safe to call if already empty.
   **/
  void clear() noexcept
  {
    if (!this->data_) {
      return;
    }
    if (this->handler_)
    {
      this->handler_->destruct((void*)this->data_);
      this->handler_ = nullptr;
    } else {
      delete [] (char*)this->data_;
    }
    this->data_ = nullptr;
  }

  /**
   * Construct from any compatible type. The argument will be moved into
   * the new Any if it supports it, and the argument is an rvalue reference.
   * Otherwise, it will be copied.
   **/
  template<typename T,
    typename std::enable_if<
      !is_type_tag<T>() &&
      !is_convertible<T, ConstAnyRef>(), int>::type = 0>
  explicit BasicOwningAny(T &&t)
    : Base(&get_type_handler(type<decay_<T>>{}),
        reinterpret_cast<void*>(
            new decay_<T>(std::forward<T>(t))))
  {}

  /**
   * Construct any compatible type in-place. The first argument is a
   * madara::type struct which provides the type as a template parameter,
   * but otherwise holds no data. The remaining arguments are forwarded to
   * that type's constructor arguments.
   *
   * Example:
   * @code{.cpp}
   * using madara::knowledge;
   *
   * // constructs a vector in-place with 42 copies of a string
   * Any a(type<std::vector<std::string>>{}, 42, "Don't panic");
   * @endcode
   **/
  template<typename T, typename... Args>
  explicit BasicOwningAny(type<T> t, Args&&... args)
    : Base(&get_type_handler(t),
        reinterpret_cast<void*>(new T(std::forward<Args>(args)...))) {}

  /**
   * Construct any compatible type in-place. The first argument is a
   * madara::type struct which provides the type as a template parameter,
   * but otherwise holds no data. The second argument is an initializer list
   * that will be passed along to that type's constructor.
   *
   * Example:
   * @code{.cpp}
   * using madara::knowledge;
   *
   * // constructs a vector in-place with several strings
   * Any a(type<std::vector<std::string>>{}, {"mostly", "harmless"});
   * @endcode
   **/
  template<typename T, typename I>
  explicit BasicOwningAny(type<T> t, std::initializer_list<I> init)
    : BasicOwningAny(t, init.begin(), init.end()) {}

  /**
   * Construct with serialized data, for lazy deserialization when first needed.
   *
   * Note that this lazy deserialization is not fully type-safe, and might not
   * throw an exception if the wrong type is used. The result may be garbled
   * data, but shouldn't segfault or trample other data.
   *
   * The first parameter is a type tag, which is available by-value from the
   * global `madara::knowledge::raw_data`.
   *
   * @param data a pointer to the serialized data to copy into this Any
   * @param size the amount of data to copy
   **/
  explicit BasicOwningAny(tags::raw_data_t, const char *data, size_t size)
    : Base(nullptr, Base::raw_data_storage::make(data, size)) {}

  /**
   * Construct any compatible type in-place. The first argument is a
   * madara::type struct which provides the type as a template parameter,
   * but otherwise holds no data. The remaining arguments are forwarded to
   * that type's constructor arguments.
   *
   * Example:
   * @code{.cpp}
   * using madara::knowledge;
   *
   * Any a;
   * // constructs a vector in-place with 42 copies of a string
   * a.emplace(type<std::vector<std::string>>{}, 42, "Don't panic");
   * @endcode
   **/
  template<typename T, typename... Args>
  T &emplace(type<T>, Args&&... args)
  {
    std::unique_ptr<T> ptr(new T(std::forward<Args>(args)...));
    const auto &handler = get_type_handler(type<decay_<T>>{});

    clear();

    this->handler_ = &handler;
    T *ret = ptr.release();
    this->data_ = reinterpret_cast<void*>(ret);
    return *ret;
  }

  /**
   * Construct any compatible type in-place. The first argument is a
   * madara::type struct which provides the type as a template parameter,
   * but otherwise holds no data. The second argument is an initializer list
   * that will be passed along to that type's constructor.
   *
   * Example:
   * @code{.cpp}
   * using madara::knowledge;
   *
   * Any a;
   * // constructs a vector in-place with several strings
   * a.emplace(type<std::vector<std::string>>{}, {"mostly", "harmless"});
   * @endcode
   **/
  template<typename T, typename I>
  T &emplace(type<T> t, std::initializer_list<I> init)
  {
    return emplace(t, init.begin(), init.end());
  }

  /**
   * Construct any compatible type in-place. The arguments are forwarded to
   * the given type's constructor arguments.
   *
   * Example:
   * @code{.cpp}
   * using madara::knowledge;
   *
   * Any a;
   * // constructs a vector in-place with 42 copies of a string
   * a.emplace<std::vector<std::string>>(42, "Don't panic");
   * @endcode
   **/
  template<typename T, typename... Args>
  T &emplace(Args&&... args)
  {
    return emplace(type<T>{}, std::forward<Args>(args)...);
  }

  /**
   * Construct any compatible type in-place. The first argument is a
   * madara::type struct which provides the type as a template parameter,
   * but otherwise holds no data. The second argument is an initializer list
   * that will be passed along to that type's constructor.
   *
   * Example:
   * @code{.cpp}
   * using madara::knowledge;
   *
   * Any a;
   * // constructs a vector in-place with several strings
   * a.emplace<std::vector<std::string>>({"mostly", "harmless"});
   * @endcode
   **/
  template<typename T, typename I>
  T &emplace(std::initializer_list<I> init)
  {
    return emplace(type<T>{}, init);
  }

  /**
   * Set from any compatible type. The argument will be moved into
   * this Any if it supports it, and the argument is an rvalue reference.
   * Otherwise, it will be copied.
   **/
  template<typename T>
  T &set(T &&t)
  {
    return emplace(type<decay_<T>>{},
           std::forward<T>(t));
  }

  /**
   * Store serialized data, for lazy deserialization when first needed.
   *
   * Note that this lazy deserialization is not fully type-safe, and might not
   * throw an exception if the wrong type is used. The result may be garbled
   * data, but shouldn't segfault or trample other data.
   *
   * @param data a pointer to the serialized data to copy into this Any
   * @param size the amount of data to copy
   **/
  void set_raw(const char *data, size_t size)
  {
    clear();
    this->data_ = Base::raw_data_storage::make(data, size);
  }

  /**
   * Store serialized data, for lazy deserialization when first needed.
   *
   * Note that this lazy deserialization is not fully type-safe, and might not
   * throw an exception if the wrong type is used. The result may be garbled
   * data, but shouldn't segfault or trample other data.
   *
   * The first parameter is a type tag, which is available by-value from the
   * global `madara::knowledge::raw_data`.
   *
   * @param data a pointer to the serialized data to copy into this Any
   * @param size the amount of data to copy
   **/
  void set(tags::raw_data_t, const char *data, size_t size)
  {
    set_raw(data, size);
  }

  /**
   * Unserialize the given type from the given character array, and store into
   * this Any. This operation provides the strong exception-guarantee: if an
   * exception is throw during unserialization, this Any will not be modified.
   **/
  template<typename K>
  size_t unserialize(K k, const char *data, size_t size)
  {
    namespace bio = boost::iostreams;

    bio::array_source input_source(data, size);
    bio::stream<bio::array_source> input_stream(input_source);

    auto pos = input_stream.tellg();
    unserialize(k, input_stream);
    auto len = input_stream.tellg() - pos;

    return len;
  }

  /**
   * Unserialize the given type from the given character array, and store into
   * this Any. This operation provides the strong exception-guarantee: if an
   * exception is throw during unserialization, this Any will not be modified.
   **/
  template<typename T>
  size_t unserialize(const char *data, size_t size)
  {
    return unserialize(type<T>{}, data, size);
  }

  /**
   * Unserialize the given type from the given input stream, and store into
   * this Any. This operation provides the strong exception-guarantee: if an
   * exception is throw during unserialization, this Any will not be modified.
   **/
  template<typename K>
  void unserialize(K k, std::istream &stream)
  {
    madara_iarchive archive(stream);

    unserialize(k, archive);
  }

  /**
   * Unserialize the given type from the given input stream, and store into
   * this Any. This operation provides the strong exception-guarantee: if an
   * exception is throw during unserialization, this Any will not be modified.
   **/
  template<typename T>
  void unserialize(std::istream &stream)
  {
    unserialize(type<T>{}, stream);
  }

  /**
   * Unserialize the given type from the given madara_iarchive, and store into
   * this Any. This operation provides the strong exception-guarantee: if an
   * exception is throw during unserialization, this Any will not be modified.
   **/
  template<typename T>
  void unserialize(type<T> t, madara_iarchive &archive)
  {
    const TypeHandlers &handler = get_type_handler(t);
    std::unique_ptr<T> ptr(new T{});

    handler.load(archive, (void*)ptr.get());

    clear();
    this->data_ = reinterpret_cast<void*>(ptr.release());
    this->handler_ = &handler;
  }

  void unserialize(const char *type, madara_iarchive &archive);

  /**
   * Unserialize the given type from the given character array, and store into
   * this Any, using saved type tag to determine type. This operation provides
   * the strong exception-guarantee: if an exception is throw during
   * unserialization, this Any will not be modified.
   *
   * Use with data serialized by tagged_serialize()
   **/
  size_t tagged_unserialize(const char *data, size_t size)
  {
    namespace bio = boost::iostreams;

    bio::array_source input_source(data, size);
    bio::stream<bio::array_source> input_stream(input_source);

    auto pos = input_stream.tellg();
    madara_iarchive archive(input_stream);
    std::string tag;
    archive >> tag;
    if (tag == "") {
      auto len = input_stream.tellg() - pos;
      set_raw(data + len, size - len);
      return size;
    } else {
      unserialize(tag.c_str(), archive);
    }
    auto len = input_stream.tellg() - pos;

    return len;
  }

  /**
   * Unserialize the given type from the given input stream, and store into
   * this Any, using saved type tag to determine type. This operation provides
   * the strong exception-guarantee: if an exception is throw during
   * unserialization, this Any will not be modified.
   *
   * Use with data serialized by tagged_serialize()
   **/
  void tagged_unserialize(std::istream &stream)
  {
    madara_iarchive archive(stream);

    tagged_unserialize(archive);
  }

  /**
   * Unserialize the given type from the given madara_iarchive, and store into
   * this Any, using saved type tag to determine type. This operation provides
   * the strong exception-guarantee: if an exception is throw during
   * unserialization, this Any will not be modified.
   *
   * Use with data serialized by tagged_serialize()
   **/
  void tagged_unserialize(madara_iarchive &archive)
  {
    std::string tag;
    archive >> tag;
    unserialize(tag.c_str(), archive);
  }

  /**
   * Unserialize the given type from the given madara_iarchive, and store into
   * this Any. This operation provides the strong exception-guarantee: if an
   * exception is throw during unserialization, this Any will not be modified.
   **/
  template<typename T>
  void unserialize(madara_iarchive &archive)
  {
    unserialize(type<T>{}, archive);
  }

  /**
   * Unserialize the given type from the given archive, and store into
   * this Any. This operation provides the strong exception-guarantee: if an
   * exception is throw during unserialization, this Any will not be modified.
   **/
  template<typename T>
  void unserialize(type<T> t, json_iarchive &archive)
  {
    const TypeHandlers &handler = get_type_handler(t);
    std::unique_ptr<T> ptr(new T{});

    handler.load_json(archive, (void*)ptr.get());

    clear();
    this->data_ = reinterpret_cast<void*>(ptr.release());
    this->handler_ = &handler;
  }

  /**
   * Unserialize the given type from the given archive, and store into
   * this Any. This operation provides the strong exception-guarantee: if an
   * exception is throw during unserialization, this Any will not be modified.
   **/
  template<typename T>
  void unserialize(json_iarchive &archive)
  {
    unserialize(type<T>{}, archive);
  }

  /**
   * Unserialize the given type from the given archive, and store into
   * this Any. This operation provides the strong exception-guarantee: if an
   * exception is throw during unserialization, this Any will not be modified.
   **/
  void unserialize(const char *type, json_iarchive &archive);

  /**
   * Access the Any's stored value by reference.
   * If empty() is true, throw BadAnyAccess exception; else,
   * If raw() is true, try to deserialize using T, and store deserialized
   * data if successful, else throw BadAnyAccess exception.
   * Otherwise, check type_id<T> matches handler_->tindex; if so,
   * return *data_ as T&, else throw BadAnyAccess exception
   *
   * Note that T must match the type of the stored value exactly. It cannot
   * be a parent or convertible type, including primitive types.
   *
   * @return a reference to the contained value
   **/
  template<typename T>
  T &ref(type<T> t)
  {
    if (!this->data_) {
      throw exceptions::BadAnyAccess("ref() called on empty Any");
    } else if (!this->handler_) {
      typename Base::raw_data_storage *sto =
        (typename Base::raw_data_storage *)this->data_;
      unserialize(t, sto->data, sto->size);
    } else if (type_id<T>() != this->handler_->tindex) {
      throw exceptions::BadAnyAccess(t, this->handler_->tindex);
    }
    return this->impl().ref_unsafe(t);
  }

  /**
   * Access the Any's stored value by reference.
   * If empty() is true, throw BadAnyAccess exception; else,
   * If raw() is true, try to deserialize using T, and store deserialized
   * data if successful, else throw BadAnyAccess exception.
   * Otherwise, check type_id<T> matches handler_->tindex; if so,
   * return the stored data as T&, else throw BadAnyAccess exception
   *
   * Note that T must match the type of the stored value exactly. It cannot
   * be a parent or convertible type, including primitive types.
   *
   * @return a reference to the contained value
   **/
  template<typename T>
  T &ref()
  {
    return ref(type<T>{});
  }

  using Base::ref;

  /**
   * Take the Any's stored value, leaving it empty. On moveable types, this
   * will not copy the value.
   *
   * If empty() is true, throw BadAnyAccess exception; else,
   * If raw() is true, try to deserialize using T, and store deserialized
   * data if successful, else throw BadAnyAccess exception.
   * Otherwise, check type_id<T> matches handler_->tindex; if so,
   * take and return the data, else throw BadAnyAccess exception
   *
   * Note that T must match the type of the stored value exactly. It cannot
   * be a parent or convertible type, including primitive types.
   *
   * @return the formerly contained value
   **/
  template<typename T>
  T take(type<T> t)
  {
    T ret(std::move(ref(t)));
    clear();
    return ret;
  }

  /**
   * Take the Any's stored value, leaving it empty. On moveable types, this
   * will not copy the value.
   *
   * If empty() is true, throw BadAnyAccess exception; else,
   * If raw() is true, try to deserialize using T, and store deserialized
   * data if successful, else throw BadAnyAccess exception.
   * Otherwise, check type_id<T> matches handler_->tindex; if so,
   * take and return the data, else throw BadAnyAccess exception
   *
   * Note that T must match the type of the stored value exactly. It cannot
   * be a parent or convertible type, including primitive types.
   *
   * @return the formerly contained value
   **/
  template<typename T>
  T take()
  {
    return take(type<T>{});
  }

private:
  template<typename T>
  static T *take_ptr(T *&in)
  {
    T *ret = in;
    in = nullptr;
    return ret;
  }

  template<typename T>
  static const T *take_ptr(const T *&in)
  {
    const T *ret = in;
    in = nullptr;
    return ret;
  }
};

/**
 * A general purpose type which can store any type which is:
 *
 *  * Default constructible
 *  * Copy constructible
 *  * Serializable by Boost.Serialization, or implements the for_each_field
 *    free function.
 *
 * This class is used by KnowledgeRecord and KnowledgeBase to store (nearly)
 * arbitrary types. This class owns the value it stores, and will automatically
 * destruct the object when the Any gets destructed, like std::unique_ptr.
 *
 * It is similar in principle to Boost.Any (and C++17 std::any), but
 * incorporates serialization support, and a different interface.
 *
 * This class cannot modify the stored object, but it can replace what object
 * is stored, which will destroy the previously stored object.
 * A `const ConstAny` cannot change at all once constructed, until the
 * `const ConstAny` itself is destructed.
 **/
class ConstAny : public BasicOwningAny<ConstAny,
  BasicConstAny<ConstAny, Any, ConstAnyRef>>
{
  using Base = BasicOwningAny<ConstAny,
    BasicConstAny<ConstAny, Any, ConstAnyRef>>;

public:
  using Base::Base;

  static ConstAny construct(const char *name) {
    return construct_const(name);
  }
};

/**
 * A general purpose type which can store any type which is:
 *
 *  * Default constructible
 *  * Copy constructible
 *  * Serializable by Boost.Serialization, or implements the for_each_field
 *    free function.
 *
 * This class is used by KnowledgeRecord and KnowledgeBase to store (nearly)
 * arbitrary types. This class owns the value it stores, and will automatically
 * destruct the object when the Any gets destructed, like std::unique_ptr.
 *
 * It is similar in principle to Boost.Any (and C++17 std::any), but
 * incorporates serialization support, and a different interface.
 *
 * Note that, a `const Any` can modify the object constructed, it simply
 * cannot change which object it holds. Use the ConstAny class to hold a const
 * object which cannot be modifid, and `const ConstAny` to hold a const
 * object that will not be destructed until the `const ConstAny` is destructed.
 **/
class Any : public BasicOwningAny<Any,
  BasicAny<Any, Any, AnyRef, ConstAnyRef>>
{
  using Base = BasicOwningAny<Any,
    BasicAny<Any, Any, AnyRef, ConstAnyRef>>;

public:
  using Base::Base;
};

template<typename T>
inline void AnyRegistry::register_type(const char *name)
{
  (void)type_builders().emplace(std::piecewise_construct,
      std::forward_as_tuple(name),
      std::forward_as_tuple([](){ return Any(type<T>{}); }));

  auto &ptr = *get_type_name_ptr<T>();
  if (ptr == nullptr) {
    ptr = name;
  }
}

inline Any AnyRegistry::construct(const char *name)
{
  auto iter = type_builders().find(name);
  if (iter == type_builders().end()) {
    throw exceptions::BadAnyAccess(std::string("Type ") + name +
        "is not registered");
  }
  return iter->second();
}

inline ConstAny AnyRegistry::construct_const(const char *name)
{
  return construct(name);
}

template<typename Impl, typename Base>
inline void BasicOwningAny<Impl, Base>::unserialize(
    const char *type, madara_iarchive &archive)
{
  Any any(construct(type));

  any.handler_->load(archive, any.data_);

  using std::swap;
  swap(this->handler_, any.handler_);
  swap(this->data_, any.data_);
}

template<typename Impl, typename Base>
inline void BasicOwningAny<Impl, Base>::unserialize(
    const char *type, json_iarchive &archive)
{
  Any any(construct(type));

  any.handler_->load_json(archive, any.data_);

  using std::swap;
  swap(this->handler_, any.handler_);
  swap(this->data_, any.data_);
}

inline ConstAnyRef::ConstAnyRef(const Any &other)
  : Base(other.handler_, other.data_) {}

inline AnyRef::AnyRef(const Any &other)
  : Base(other.handler_, other.data_) {}

} // namespace knowledge

} // namespace madara

#endif  // MADARA_KNOWLEDGE_ANY_H_

#include "DefaultTypeHandlers.h"
