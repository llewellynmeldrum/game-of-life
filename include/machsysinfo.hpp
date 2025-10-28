#include <sstream>
#include <stdint.h>

#include <vector>
#include <sys/types.h>

#include <stdlib.h>
#include <type_traits>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sys/sysctl.h>
#include <mach/machine.h>

using int32 = int32_t;
using int64 = int64_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using std::vector;
using std::string;

template <typename T>
static inline double btokb(T b) {
	if constexpr(std::is_arithmetic_v<T>) {
		return b / 1'000.0;
	} else {
		return 0;
	}
}


// what would be a nice interface?
// Ideally, you define the fields of some type, give them a string, and maybe a column width. Then, upon calling a function,
// all of the fields are printed. The fields values should also be acccesible through some internal data member, though this is difficult to get right.

static inline size_t sysctl_buf_sz_byname(const char* sysctl_name) {
	size_t buf_sz = 0;
	if (sysctlbyname(sysctl_name, NULL, &buf_sz, NULL, 0)) {
		return -1;
	}
	return buf_sz;
}

template <typename T>
struct Field {
	string sysctl_id;
	string name;
	T val;
	string suffix;

	Field(const char* sysctl_name, string preferred_name, string suffix) {
		this->sysctl_id = std::string(sysctl_name);
		name = preferred_name;
		this->suffix = suffix;

		size_t buf_sz = sysctl_buf_sz_byname(sysctl_name);
		void *buf = malloc(buf_sz);

		int err = sysctlbyname(sysctl_name, buf, &buf_sz, NULL, 0);
		if (err) {
			perror("sysctlbyname in Field ctor");
			fprintf(stderr, "offending name: '%s'\n", sysctl_name);

		}
		if constexpr(std::is_same_v<T, std::string>) {
			val = std::string((char*)buf, buf_sz);
			return;
		}
		if (!err) val = *(T * )buf;

		free(buf);

		// handle special cases here, like byteorder
	}

	Field(const char* sysctl_name)
		: Field(sysctl_name, sysctl_name) {}

	Field(const char* sysctl_name, string preferred_name):
		Field(sysctl_name, preferred_name, "") { }


	auto get() {
		return val;
	}
	string to_string() {
		std::ostringstream oss;
		oss << std::left << std::setw(20) << name << ":";

		using D = std::decay<T>;
		// depending on the type, evaluate special cases
		if constexpr(std::is_same_v<T, std::string>) {
			oss << val;
		} else  {
			oss << to_string_arithmetic_val();
		}
		oss << suffix;
		return oss.str();
	}
  private:
	string to_string_arithmetic_val() {
		std::ostringstream val_ss;
		if (sysctl_id == std::string("hw.byteorder")) {
			if (val == 1234) val_ss << "LITTLE ENDIAN";
			else if (val == 4321) val_ss << "big endian";
		} else if (sysctl_id == std::string("hw.memsize")) {
			val_ss << std::setprecision(4) << (double)val / 1'000'000'000;
			suffix = "GB";
		} else if ( sysctl_id == std::string("hw.pagesize") 	||
		            sysctl_id == std::string("hw.l1dcachesize") ||
		            sysctl_id == std::string("hw.l1icachesize")) {
			val_ss << std::setprecision(4) << (double)val / 1'000;
			suffix = "KB";
		} else if (sysctl_id == std::string("hw.l2cachesize")) {
			val_ss << std::setprecision(3) << (double)val / 1'000'000;
			suffix = "MB";
		} else if (sysctl_id == std::string("hw.cputype")) {
			if (val & CPU_TYPE_ARM64) {
				val_ss << "arm64";
			} else if (val & CPU_TYPE_X86_64) {
				val_ss << "X86_64";
			} else {
				val_ss << "Your cpu isnt arm64 or x86. How did you even get this far\n";
			}
		} else if (sysctl_id == std::string("hw.cpusubtype")) {
		// *INDENT-OFF*
			switch (val) {
			case CPU_SUBTYPE_ARM64_V8:val_ss << "arm64_v8"; break;
			case CPU_SUBTYPE_ARM64E:val_ss << "arm64e" 	; break;
			default: val_ss << "unknown subtype. Only defined for arm64 type cpus";
			}

		// *INDENT-ON*
		} else {
			val_ss << val;
		}

		return val_ss.str();
	}


};

class SystemInfo {
	// memory info
	Field<int64>  	memsize          = Field<int64>  ("hw.memsize"		, "Memory size");
	Field<int64>  	pagesize         = Field<int64>  ("hw.pagesize"		, "Page Size", "B");
	Field<int64>   	cachelinesize	 = Field<int64>  ("hw.cachelinesize"	, "Cache Line Size", "B");
	Field<int64>   	l1dcachesize     = Field<int64>  ("hw.l1dcachesize"	, "L1 Data Cache Size", "B");
	Field<int64>   	l1icachesize     = Field<int64>  ("hw.l1icachesize"	, "L1 Inst Caceh Size", "B");
	Field<int64>   	l2cachesize      = Field<int64>  ("hw.l2cachesize"	, "L2 Cache Size", "B");

	// cpu info
	Field<int32> 	activecpu 	 = Field<int32>	 ("hw.activecpu"	, "Current CPU idx");
	Field<int32>   	cputype          = Field<int32>  ("hw.cputype"		, "CPU Type");
	Field<int32>   	cpusubtype       = Field<int32>  ("hw.cpusubtype"	, "CPU Sub-type");
	Field<int32>   	byteorder        = Field<int32>  ("hw.byteorder"	, "Endianness");
	Field<int32>   	logicalcpu       = Field<int32>  ("hw.logicalcpu"	, "N Logical cpus");
	Field<int32>  	physicalcpu      = Field<int32>  ("hw.physicalcpu"	, "N Physical cpus");
	Field<int32>   	logicalcpu_max   = Field<int32>  ("hw.logicalcpu_max"	, "N logical cpu max");
	Field<int32>   	cpu64bit	 = Field<int32>   ("hw.cpu64bit_capable"	, "Is cpu 64bit?");

	// system info
	Field<string> 	model   	 = Field<string> ("hw.model"		, "Model");
	Field<string>   machine   	 = Field<string> ("hw.machine"		, "Machine");
	Field<int32>   	packages         = Field<int32>  ("hw.packages"		, "Packages?");
	Field<int64>  	tbfrequency      = Field<int64>  ("hw.tbfrequency"	, "Timebase freq", "/s");

  public:
	void print_all() {
		std::cout << std::endl << "MACHINE:"                 << std::endl;
		std::cout << model                      .to_string() << std::endl;
		std::cout << machine                    .to_string() << std::endl;
		std::cout << packages                   .to_string() << std::endl;
		std::cout << tbfrequency                .to_string() << std::endl;

		std::cout << std::endl << "MEMORY:"	<< std::endl;
		std::cout << memsize                    .to_string() << std::endl;
		std::cout << pagesize                   .to_string() << std::endl;
		std::cout << cachelinesize              .to_string() << std::endl;
		std::cout << l1dcachesize               .to_string() << std::endl;
		std::cout << l1icachesize               .to_string() << std::endl;
		std::cout << l2cachesize                .to_string() << std::endl;

		std::cout << std::endl << "CPU:" << std::endl;
		std::cout << activecpu 	                .to_string() << std::endl;
		std::cout << cputype                    .to_string() << std::endl;
		std::cout << cpusubtype                 .to_string() << std::endl;
		std::cout << byteorder                  .to_string() << std::endl;
		std::cout << cpu64bit                   .to_string() << std::endl;
		std::cout << logicalcpu                 .to_string() << std::endl;
		std::cout << physicalcpu                .to_string() << std::endl;
		std::cout << logicalcpu_max             .to_string() << std::endl;

	}

	void query_sysctl();
};

