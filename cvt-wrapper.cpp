// a tool to accompany cvt-export, generating memblk/memfile wrappers for raw resources
// argv: source, header, ("file" | "export", exp-head), files...
// note: if using export, the exported header must be included prior

#include <algorithm>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>

#include "include/fmt.hpp"

std::string fname_to_ident(std::string s)
{
	size_t last_sep = s.find_last_of("\\/", s.size() - 1);
	s = last_sep == std::string::npos ? s : s.substr(last_sep + 1);
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

void output_using_memfile(int filec, char** filev)
{
	for(int i = 0; i < filec; ++i) {
		auto ident = fname_to_ident(filev[i]);
		fmt::print(o_head, "\textern res::ro_memfile {};\n", ident);
		fmt::print(o_src, "\tres::ro_memfile {}(\"{}\");\n", ident, filev[i]);
	}
}

void output_using_memblk(int filec, char** filev)
{
	for(int i = 0; i < filec; ++i) {
		auto ident = fname_to_ident(filev[i]);
		fmt::print(o_head, "\textern res::ro_memblk {};\n", ident);
		fmt::print(o_src, "\tres::ro_memblk {}({});\n", ident, "res0_" + ident);
	}
}

void output_type(bool use_memfile, int argc, char** argv)
{
	if(!use_memfile) {
		fmt::print(o_src, "#include \"{}\"\n", argv[4]);
	}

	fmt::print(o_src,
R"(#include "{}"
// auto-generated source file from cvt-wrapper
// do not directly modify

namespace store {{

)", argv[2]);

	fmt::print(o_head,
R"(#pragma once
// auto-generated source file from cvt-wrapper
// do not directly modify

#include "res/{}"

namespace store {{

)", use_memfile ? "memfile.hpp" : "memblk.hpp");

	if(use_memfile) {
		output_using_memfile(argc - 4, argv + 4);
	} else {
		output_using_memblk(argc - 5, argv + 5);
	}

	fmt::print(o_src, "\n}}");
	fmt::print(o_head, "\n}}");
}

int main(int argc, char** argv)
{
	if(argc <= 3) {
		fmt::print(std::cerr, "{}: {}\n", argv[0], "insufficient arguments");
		return 1;
	}

	bool use_memfile;
	if(argv[3] == std::string("file")) {
		use_memfile = true;
	} else if(argv[3] == std::string("export")) {
		use_memfile = false;
	} else {
		fmt::print(std::cerr, "{}: {}: {}\n", argv[0], argv[3], "invalid type");
		return 1;
	}

	if(!use_memfile && argc <= 4) {
		fmt::print(std::cerr, "{}: {}\n", argv[0], "insufficient arguments");
		return 1;
	}

	std::cout << "Creating for type " << argv[3] << '\n'
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

	output_type(use_memfile, argc, argv);

}
