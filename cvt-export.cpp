// an internal tool to convert raw resources (e.g .images) to files
// argv: source, header, project name, files...

#include <algorithm>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <cerrno>
#include <cstring>

#include "include/fmt.hpp"
#include "res/memfile.hpp"

std::string fname_to_ident(std::string s)
{
	size_t last_sep = s.find_last_of("\\/", s.size() - 1);
	s = "res0_" + (last_sep == std::string::npos ? s : s.substr(last_sep + 1));
	auto xform_fn = [](char c) {
		if(!(std::isalnum(c, std::locale::classic()))) {
			return '_';
		}
		return std::tolower(c, std::locale::classic());
	};

	std::transform(s.begin(), s.end(), s.begin(), xform_fn);
	return s;
}

std::fstream o_src, o_head;

int main(int argc, char** argv)
{
	if(argc <= 3) {
		fmt::print(std::cerr, "{}: {}\n", argv[0], "insufficient arguments");
		return 1;
	}

	std::cout << "Creating for project " << argv[3] << '\n'
		<< "src: " << argv[1] << '\n'
		<< "hdr: " << argv[2] << '\n';

	o_src.open(argv[1], std::ofstream::out | std::ofstream::trunc);
	if(!o_src) {
		fmt::print(std::cerr, "{}: {}: {}\n", argv[0], argv[1], std::strerror(errno));
		return 1;
	}


	o_head.open(argv[2], std::ofstream::out | std::ofstream::trunc);
	if(!o_src) {
		fmt::print(std::cerr, "{}: {}: {}\n", argv[0], argv[2], std::strerror(errno));
		return 1;
	}

	fmt::print(o_src,
R"(#include "{}"
// auto-generated source file from cvt-export
// do not directly modify

extern "C" {{

)", argv[2]);


	fmt::print(o_head,
R"(#pragma once
// auto-generated header file from cvt-export
// do not directly modify

#ifdef {}_EXPORTS
#define EXPORTS
#endif

#include "res/dllport.hpp"
#include "include/types.hpp"

extern "C" {{

)", argv[3]);

	for(int i = 4; i < argc; ++i) {
		res::ro_memfile raw_file;
		std::error_code ec;

		if(!raw_file.open(argv[i], ec)) {
			fmt::print(std::cerr, "{}: {}: {}\n", argv[0], argv[i], ec.message());
		}

		auto ident = fname_to_ident(argv[i]);

		fmt::print(o_head, "\t_dll_api_ byte_block<{}> {};\n", raw_file.size(), ident);
		fmt::print(o_src,  "\t_dll_api_ byte_block<{}> {} = {{", raw_file.size(), ident);

		size_t counter = 0;
		for(auto ptr = static_cast<const unsigned char*>(raw_file.get()), end = ptr + raw_file.size();
		    ptr != end; ++ptr, ++counter) {
			if(counter % 32 == 0) {
				o_src << '\n';
				counter = 0;
			}
			fmt::print(o_src, " {:>3},", *ptr);
		}

		fmt::print(o_src, "\n\t}};\n\n");
	}

	fmt::print(o_head, "\n}}\n#ifdef EXPORTS\n#undef EXPORTS\n#endif");
	fmt::print(o_src, "}}");

}
