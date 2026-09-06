#pragma once
// SPDX-FileCopyrightText: © 2017-2026 Adrian Johnston.
// SPDX-License-Identifier: MIT
// This file is licensed under the MIT license found in the LICENSE.md file.

/// \file
/// Single-ownership C++ RAII abstraction for file I/O. Both POSIX and libc
/// implementations are available.

#include "libhatchet.h"

// HX_USE_MACROS_WITH_MODULE allows including macros alongside the module.
#if HX_USE_MACROS_WITH_MODULE
#error Header does not provide macros alone.
#endif

#if HX_USE_FILE_IO

#if HX_CPLUSPLUS >= 202302L
#include "hxexpected.hpp"
#endif

HX_NS_BEGIN_

class hxfile;

/// `hxin` - Global reference to stdin or equivalent.
extern hxfile hxin;

/// `hxout` - Global reference to stdout or equivalent.
extern hxfile hxout;

/// `hxerr` - Global reference to stderr or equivalent.
extern hxfile hxerr;

/// `hxdev_null` - Global equivalent to `/dev/null`. May be written to but not
/// read from.
extern hxfile hxdev_null;

/// `hxendl` - Equivalent to `std::endl` without the flush. Does not vary by
/// platform. Non-empty POSIX text files must end with `\n`.
hxinline_constexpr char hxendl[] = "\n";

/// `hxfile` - Single-ownership C++ RAII abstraction for file I/O. Provides a
/// mixture of unformatted binary stream operations and formatted
/// `printf`/`scanf` style I/O, along with optional error handling. `gcc` is
/// useful for validating `printf`/`scanf` style arguments. However,
/// memory-imaged data structures are still recommended. `hxfile` uses binary
/// I/O only for portability. It implements the equivalent of the standard
/// `eofbit` and `failbit` but not the `badbit`. The `failbit` should always be
/// set when the `eofbit` is set.
///
/// Here is the syntax to make a block of code conditional on opening a file.
/// The filename is also formatted printf style. This is equivalent to Python's
/// `with open(filename, mode) as f:`.
///
/// ```cpp
/// if(hxfile f=hxfile(hxfile::open_mode_in, "pkg%d.bin", i)) {
///   f >> manifest; // binary read.
///   // ...
/// }
/// ```
///
/// To switch to a different implementation, use an alternate `.cpp` file for
/// your target. Allows `hxerr` to be a serial port while file I/O uses a DMA
/// controller.
///
/// WARNING: `get_pos`/`set_pos` over 2 GiB is not supported on Windows. As
/// well, `size_t` is limited to 2 GiB on all 32-bit platforms.
class hxfile {
public:
	/// `open_mode` - Flags indicating how the file is to be used. Modifying or
	/// appending to an existing file is not implemented.
	enum open_mode : uint8_t {
		/// No flags.
		open_mode_none = 0u,
		/// Open for binary reading. e.g., `"rb"`.
		open_mode_in = 1u,
		/// Open for binary writing. Replaces any existing file with an empty
		/// one even if `open_mode_in` is used at the same time. e.g., `"wb"`.
		open_mode_out = 2u,
		/// By default, any unexpected failure is reported only through the
		/// failure flag. To assert on reasonably unforeseen failures, set
		/// `open_mode_asserts`. Bad parameters (e.g., providing a null buffer)
		/// will still result in assertions regardless of this flag.
		open_mode_asserts = 4u
	};

	/// Default-constructs as a closed file.
	hxfile(void);

	/// Constructs and opens a file with a formatted filename. Uses a
	/// non-standard argument order.
	/// - `mode` : Combination of `open_mode` flags describing how to open the
	///   file.
	/// - `filename` : Non-null `printf`-style format string naming the file.
	/// - `...` : Additional arguments matching the `filename` format
	///   specifiers.
	hxfile(uint8_t mode_, const char* filename_, ...) hxattr_printf(3, 4);

	/// Constructs the file object with an unowned implementation-specific
	/// handle and a specific mode. Performs no checks. Use `hxin`, `hxout`,
	/// `hxerr`, and `hxdev_null` instead.
	/// - `mode` : Combination of `open_mode` flags describing the handle.
	/// - `file` : Implementation-specific handle.
	hxfile(uint8_t mode_, intptr_t file_);

	/// Disallow usage where the filename comes first, like with `fopen`.
	/// This is done because `hxfile::hxfile()` uses variadic arguments.
	hxfile(const char* file_, uint8_t mode_=0) = delete;

	/// Move constructor. No copy constructor is provided.
	hxfile(hxfile&& file_) noexcept;

	/// Destroys the file and ensures it is closed when the object goes out of
	/// scope.
	~hxfile();

	/// Move assignment. No copy assignment operator is provided.
	void operator=(hxfile&& file_) noexcept;

	/// Checks if the file is open, `EOF` has not been reached, and no error has
	/// been encountered. See usage example in the class documentation.
	operator bool(void) const;

	/// Opens a file with the specified mode and formatted filename.
	/// - `mode` : Combination of `open_mode` flags describing how to open the
	///   file.
	/// - `filename` : Non-null `printf`-style format string naming the file.
	/// - `...` : Additional arguments matching the `filename` format
	///   specifiers.
	bool open(uint8_t mode_, const char* filename_, ...) hxattr_printf(3, 4);

	/// Closes the currently open file.
	void close(void);

	/// Checks if the file is open.
	hxattr_nodiscard bool is_open(void) const;

	/// Checks if an error has been encountered, EOF set or `set_fail` called.
	hxattr_nodiscard hxinline bool fail(void) const { return m_fail_; }

	/// Marks the file as having encountered a failure. Allows the user to
	/// report additional errors without having to track them. Non-standard.
	void set_fail(void) { m_fail_ = true; }

	/// Checks if `EOF` has been reached.
	hxattr_nodiscard hxinline bool eof(void) const { return m_eof_; }

	/// Resets the failure and `EOF` flags. This is required to clear `EOF`
	/// after `EOF` is encountered.
	void clear(void);

	/// Returns the current open mode of the file.
	hxattr_nodiscard hxinline uint8_t mode(void) const { return m_open_mode_; }

	/// Returns the current position in the file if open, 0 otherwise. FILE*
	/// implementation requires a 64-bit long to support 64-bit files.
	hxattr_nodiscard size_t get_pos(void) const;

	/// Sets the current position in the file. Returns true on success. FILE*
	/// implementation requires a 64-bit long to support 64-bit files. Resets
	/// the failure flag to false on success.
	bool set_pos(size_t position_);

	/// Reads a specified number of bytes from the file into the provided
	/// buffer. Does not reset the failure flag to false on success.
	/// - `bytes` : Non-null pointer to a buffer of at least `buffer_size`
	///   bytes.
	/// - `buffer_size` : Capacity of the buffer in bytes.
	/// - `count` : Number of bytes to read from the file. Must not exceed
	///   `buffer_size`.
	size_t read(void* bytes_, size_t buffer_size_, size_t count_) hxattr_nonnull(2) hxattr_hot;

	/// Writes a specified number of bytes from the provided buffer to the file.
	/// Writing will be skipped when using `hxdev_null`. Resets the failure flag
	/// to false on success.
	/// - `bytes` : Non-null pointer to a buffer that provides at least `count`
	///   bytes.
	/// - `count` : Number of bytes to write to the file.
	size_t write(const void* bytes_, size_t count_) hxattr_nonnull(2) hxattr_hot;

	/// Flushes buffered output to the underlying file. Safe to call on
	/// `hxdev_null`. Does not reset the failure flag to false on success.
	bool flush(void) hxattr_hot;

	/// Writes a formatted UTF-8 string to the file. Uses `printf` conventions.
	/// Formatting and writing will be skipped when using `hxdev_null`. Does not
	/// modify the failure flag because it is not clear from `vfprintf`.
	/// - `format` : Non-null `printf`-style format string.
	/// - `...` : Additional arguments that satisfy the format string.
	bool print(const char* format_, ...) hxattr_printf(2, 3) hxattr_hot;

	/// Reads a single unformatted native-endian object from the file.
	/// - `t` : Reference to the object where the data will be stored.
	template<typename T_>
	hxinline bool read1(T_& t_) { return this->read(&t_, sizeof t_, sizeof t_) == sizeof t_; }

#if HX_CPLUSPLUS >= 202302L
	/// Reads a single unformatted native-endian object from the file,
	/// returning it or an error if the read failed. WARNING: UB.
	template<typename T_>
	hxattr_nodiscard hxinline hxexpected<T_> expect(void) {
		T_ t_;
		const bool error_ = this->read(&t_, sizeof(T_), sizeof(T_)) != sizeof(T_);
		return hxexpected<T_>(error_, t_);
	}
#endif

	/// Writes a single unformatted native-endian object to the file.
	/// - `t` : Reference to the object containing the data to write.
	template<typename T_>
	hxinline bool write1(const T_& t_) { return this->write(&t_, sizeof t_) == sizeof t_; }

	/// Reads a single unformatted native-endian object from a stream.
	/// - `t` : Reference to the object where the data will be stored.
	template<typename T_>
	hxinline hxfile& operator>>(T_& t_) {
		this->read(&t_, sizeof t_, sizeof t_);
		return *this;
	}

	/// Writes a single unformatted native-endian object to a stream.
	/// - `t` : Reference to the object containing the data to write.
	template<typename T_>
	hxinline hxfile& operator<<(const T_& t_) {
		this->write(&t_, sizeof t_);
		return *this;
	}

	/// Writes a string literal to the file. Supports Google Test style
	/// diagnostic messages in `hxtest`.
	/// - `str` : Reference to a string literal to write to the file.
	template<size_t string_length_>
	hxinline hxfile& operator<<(const char(&str_)[string_length_]) {
		this->write(str_, string_length_-1);
		return *this;
	}

private:
	hxfile(const hxfile&) = delete;
	void operator=(const hxfile&) = delete;
	template<typename T_> hxfile& operator>>(const T_* t_) = delete;

	bool openv_(uint8_t mode_, const char* format_, va_list args_);

	intptr_t m_file_pimpl_;
	uint8_t  m_open_mode_;
	bool     m_owns_;
	bool     m_fail_;
	bool     m_eof_;
};

HX_NS_END_
#endif // HX_USE_FILE_IO
