//
//  main.cpp
//  dmpdiff
//
//  Created by udif on 11/5/16.
//  Copyright © 2016 udif. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <codecvt>
#include <locale>

#include "third_party/diff_match_patch/dmpdiff/getopt.h"
#include "third_party/diff_match_patch/diff_match_patch.h"

std::wstring readFile(std::string filename)
{
    std::wifstream wif(filename);
    wif.imbue(std::locale::global(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>)));
    std::wstringstream wss;
    wss << wif.rdbuf();
    return wss.str();
}

//std::ostream& operator<<(std::ostream& os, const Diff& d)
static void dump_Diff(const Diff& d)
{
    switch(d.operation) {
        case INSERT:
            std::wcout << "+" << d.text;
            break;
        case DELETE:
            std::wcout << "-" << d.text;
            break;
        case EQUAL:
            std::wcout << "=" << d.text;
            break;
    }
}

static void dump_diff(std::list<Diff> ld)
{
    for (auto diff : ld) {
        dump_Diff(diff);
    }
}
int main(int argc, char * argv[])
{
	bool color = true;
	bool normal = false;
	int context = 3;
	int unified = 3;

    /*
     * Sanity checking
     */
	GetOpt::GetOptResultAndArgs results;
    try {
		results = GetOpt::getopt(argc, argv,
			"c|color", "Use ANSI color diff output mode", &color,
			"n|normal", "Use normal diff output format", &normal,
			"context", "Use context diff output format", &context,
			"unified", "Use unified diff output format", &unified);
		if (results.result.helpWanted) {
			GetOpt::defaultGetoptPrinter("Usage: ", results.result.options);
			return 0;
		}
		if (results.args.size() != 2) {
			std::cerr << "Must have 2 filename arguments\n";
			exit(1);
		}

        diff_match_patch dmp;
        std::wstring wstr1 = readFile(results.args[0]);
        std::wstring wstr2 = readFile(results.args[1]);
        
        std::list<Diff> result = dmp.diff_main(wstr1, wstr2);
        dump_diff(result);
        //dmp_diff_print_raw(stdout, diff);
        //dmp_diff_free(diff);
    }
	catch (GetOpt::GetOptException e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	// insert code here...
    return 0;
}





