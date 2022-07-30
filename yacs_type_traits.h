#ifndef YACS_TYPE_TRAITS_H
#define YACS_TYPE_TRAITS_H

#include <type_traits>
#include <string_view>

namespace yacs {
  namespace detail {
    // https://stackoverflow.com/questions/45948835/stdis-base-of-and-virtual-base-class
    template <typename From, typename To, typename = void>
    struct can_static_cast : public std::false_type {};

    template <typename From, typename To>
    struct can_static_cast<From, To, std::void_t<decltype(static_cast<To>(std::declval<From>()))>> : public std::true_type {};

    template <typename Base, typename Derived>
    struct is_virtual_base_of : public std::conjunction<
        std::is_base_of<Base, Derived>,
        std::negation<can_static_cast<Base*, Derived*>>
    >{};

    /// Simple type introspection without RTTI.
    template <typename T>
    constexpr std::string_view get_type_name() {
      //std::cout << __PRETTY_FUNCTION__ << "\n";
      //std::cout << __FUNCSIG__ << "\n";
#if defined(_MSC_VER)
      constexpr std::string_view start_char_seq = "get_type_name<";
      constexpr std::string_view end_char_seq = ">(void)";
      constexpr std::string_view function_type_pattern = ")(";
      constexpr std::string_view sig = __FUNCSIG__;
      constexpr size_t sig_size = sig.size()+1;
      constexpr size_t str_seq_name_start = sig.find(start_char_seq) + start_char_seq.size();
      constexpr size_t end_of_char_str = sig.rfind(start_char_seq);
      constexpr size_t count = sig_size - str_seq_name_start - end_char_seq.size() - 1; // отстается символ '>' в конце
      constexpr std::string_view substr = sig.substr(str_seq_name_start, count);
      if constexpr (substr.find(function_type_pattern) == std::string_view::npos) {
        constexpr std::string_view class_char_seq = "class ";
        constexpr std::string_view struct_char_seq = "struct ";
        const size_t class_seq_start = substr.find(class_char_seq);
        const size_t struct_seq_start = substr.find(struct_char_seq);
        if constexpr (class_seq_start == 0) return substr.substr(class_char_seq.size());
        if constexpr (struct_seq_start == 0) return substr.substr(struct_char_seq.size());;
      }
      return substr;
#elif defined(__clang__)
      constexpr std::string_view sig = __PRETTY_FUNCTION__;
      constexpr std::string_view start_char_seq = "T = ";
      constexpr size_t sig_size = sig.size()+1;
      constexpr size_t str_seq_name_start = sig.find(start_char_seq) + start_char_seq.size();
      constexpr size_t end_of_char_str = 2;
      constexpr size_t count = sig_size - str_seq_name_start - end_of_char_str;
      return sig.substr(str_seq_name_start, count);
#elif defined(__GNUC__)
      constexpr std::string_view sig = __PRETTY_FUNCTION__;
      constexpr std::string_view start_char_seq = "T = ";
      constexpr size_t sig_size = sig.size()+1;
      constexpr size_t str_seq_name_start = sig.find(start_char_seq) + start_char_seq.size();
      constexpr size_t end_of_char_str = sig_size - sig.find(';');
      constexpr size_t count = sig_size - str_seq_name_start - end_of_char_str;
      return sig.substr(str_seq_name_start, count);
#else
#error Compiler not supported for demangling
#endif
    }

    constexpr uint8_t to_u8(const char c) { return uint8_t(c); }

    constexpr uint64_t U8TO64_LE(const char* data) {
      return  uint64_t(to_u8(data[0]))        | (uint64_t(to_u8(data[1])) << 8)  | (uint64_t(to_u8(data[2])) << 16) |
            (uint64_t(to_u8(data[3])) << 24) | (uint64_t(to_u8(data[4])) << 32) | (uint64_t(to_u8(data[5])) << 40) |
            (uint64_t(to_u8(data[6])) << 48) | (uint64_t(to_u8(data[7])) << 56);
    }

    constexpr uint64_t U8TO64_LE(const uint8_t* data) {
      return  uint64_t(data[0])        | (uint64_t(data[1]) << 8)  | (uint64_t(data[2]) << 16) |
            (uint64_t(data[3]) << 24) | (uint64_t(data[4]) << 32) | (uint64_t(data[5]) << 40) |
            (uint64_t(data[6]) << 48) | (uint64_t(data[7]) << 56);
    }

    constexpr uint64_t to_u64(const char c) { return uint64_t(to_u8(c)); }

    constexpr uint64_t murmur_hash64A(const std::string_view& in_str, const uint64_t seed) {
      constexpr uint64_t m = 0xc6a4a7935bd1e995LLU;
      constexpr int r = 47;
      const size_t len = in_str.size();
      const size_t end = len - (len % sizeof(uint64_t));

      uint64_t h = seed ^ (len * m);

      for (size_t i = 0; i < end; i += 8) {
        uint64_t k = U8TO64_LE(&in_str[i]);
        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
      }

      const auto key_end = in_str.substr(end);
      const int left = len & 7;
      switch (left) {
        case 7: h ^= to_u64(key_end[6]) << 48; [[fallthrough]];
        case 6: h ^= to_u64(key_end[5]) << 40; [[fallthrough]];
        case 5: h ^= to_u64(key_end[4]) << 32; [[fallthrough]];
        case 4: h ^= to_u64(key_end[3]) << 24; [[fallthrough]];
        case 3: h ^= to_u64(key_end[2]) << 16; [[fallthrough]];
        case 2: h ^= to_u64(key_end[1]) << 8;  [[fallthrough]];
        case 1: h ^= to_u64(key_end[0]);
          h *= m;
      };

      h ^= h >> r;
      h *= m;
      h ^= h >> r;

      return h;
    }
  }

  constexpr uint64_t default_murmur_seed = 14695981039346656037ull;
  constexpr uint64_t murmur_hash64A(const std::string_view &in_str, const uint64_t seed) {
    return detail::murmur_hash64A(in_str, seed);
  }

  constexpr uint64_t string_hash(const std::string_view &in_str) {
    return murmur_hash64A(in_str, default_murmur_seed);
  }

  template <typename T>
  constexpr std::string_view type_name() {
    return detail::get_type_name<T>();
  }

  template <typename T>
  constexpr uint64_t type_id() {
    return string_hash(type_name<T>());
  }
}

#endif
