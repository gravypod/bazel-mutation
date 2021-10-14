/**
 * getopt.h
 * Provides GNU-compliant getopt interface with template magic for a nice interface
 * Based on D's [std.getopt](https://github.com/D-Programming-Language/phobos/blob/master/std/getopt.d)
 * Authors: Erich Gubler, erichdongubler@gmail.com
 * 
 * Copyright (c) 2015 by Erich Gubler
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef GETOPT_H
#define GETOPT_H

// Core functionality
#include <iostream>// ostream for help printing
#include <sstream> // conversions between most types
#include <stdexcept> // runtime_error
#include <string>
#include <type_traits> // is_integral, // remove_const
#include <typeinfo> // use typeid(...) for error message for conversions
#include <unordered_set> // opts matching
#include <utility> // move
#include <vector>

// Case conversion
#include <cctype> // tolower
#include <algorithm> // transform

// String case conversions
#include <algorithm> // transform

// Help printing
#include <iomanip>

namespace GetOpt
{
	class GetOptException : public std::runtime_error
	{
	public:
		GetOptException(const std::string& s) : std::runtime_error(s){}
	};

	// Conforms to [D's config values](http://dlang.org/phobos/std_getopt.html#.config)
	enum class config
	{
		caseSensitive,
		caseInsensitive,
		passThrough,
		noPassThrough,
		bundling,
		noBundling,
		stopOnFirstNonOption,
		keepEndOfOptions,
		required,
	};

	class GetOptConfiguration
	{
	public:
		bool passThrough = false;
		bool bundling = false;
		bool required = false;
		bool keepEndOfOptions = false;
		bool caseSensitive = false;
		bool stopOnFirstNonOption = false;

		void set(config configOption)
		{
			switch(configOption)
			{
				case config::caseInsensitive:
					this->caseSensitive = true;
					break;
				case config::caseSensitive:
					this->caseSensitive = true;
					break;
				case config::passThrough:
					this->passThrough = true;
					break;
				case config::noPassThrough:
					this->passThrough = false;
					break;
				case config::bundling:
					this->bundling = true;
					break;
				case config::noBundling:
					this->bundling = false;
					break;
				case config::required:
					this->required = true;
					break;
				case config::keepEndOfOptions:
					this->keepEndOfOptions = true;
					break;
				case config::stopOnFirstNonOption:
					this->stopOnFirstNonOption = true;
					break;
			}
		}
	};

	class Option
	{
	private:
		Option(std::unordered_set<char>&& sos, std::unordered_set<std::string>&& los)
			: shortOpts(sos), longOpts(los){}
	public:
		bool isIncremental = false;
		std::string spec;
		std::string help;
		std::string longOptForHelp;
		std::string shortOptForHelp;
		std::unordered_set<char> shortOpts;
		std::unordered_set<std::string> longOpts;

		Option(std::string optSpec, const std::string& h = "")
			: spec(optSpec), help(h)
		{
			// std::cout << "optSpec: " << optSpec << std::endl;
			auto specSize = optSpec.size();
			if(specSize == 0)
				// Currently, D's getopt crashes on this case
				throw std::logic_error("Spec size must be greater than 0");
			// Check if incremental opt
			if(optSpec[specSize - 1] == '+')
			{
				isIncremental = true;
				optSpec.erase((specSize--) - 1);
			}

			decltype(this->shortOpts) shortOpts;
			decltype(this->longOpts) longOpts;
			// Split into long and short opts
			std::size_t found = optSpec.find('|');
			do
			{
				auto optMatch = optSpec.substr(0, found);
				switch(optMatch.size())
				{
					case 0:
						break;
					case 1:
						if(shortOpts.size() == 0)
							shortOptForHelp = optMatch[0];
						shortOpts.insert(optMatch[0]);
						break;
					default:
						if(longOpts.size() == 0)
							longOptForHelp = optMatch;
						longOpts.insert(optMatch);
						break;
				}
				if(found != std::string::npos)
				{
					optSpec = optSpec.substr(found + 1);
					found = optSpec.find('|');
				}
				else
					break;
			}
			while(true);
			if(!shortOpts.size() && !longOpts.size())
				throw std::logic_error("Cannot define blank option");
			this->shortOpts = std::move(shortOpts);
			this->longOpts = std::move(longOpts);
		}
	};

	struct GetOptResult
	{
	public:
		std::vector<Option> options;
		bool helpWanted = false;

		// Nonstandard
		bool parsing = true;

		GetOptResult() = default;
		// TODO: Define move semantics
		// Problem: Was getting error with the following commented out:
		// GetOptResult(GetOptResult&& r)
		// 	: options(std::move(r.options))
		// 	, helpWanted(r.helpWanted)
		// 	, parsing(r.parsing)
		// {}
		// GetOptResult operator=(GetOptResult&& r)
		// {
		// 	this->options = std::move(r.options);
		// 	this->helpWanted = r.helpWanted;
		// 	this->parsing = r.parsing;
		// }

		bool definedOption(const std::string& s)
		{
			if(s.size() == 1)
				return definedOption(s[0]);
			for(auto& o : options)
				if(o.longOpts.count(s))
					return true;
			return false;
		}
		bool definedOption(const char c)
		{
			for(auto& o : options)
				if(o.shortOpts.count(c))
					return true;
			return false;
		}
	};

	// Assignments from captured flags

	template<typename T>
	void throwConversionException(const std::string& s)
	{
		//TODO: Way to get user-friendly type name? typeid(...)?
		throw GetOptException("\"" + s + "\" is not convertible to type "
									+ typeid(T).name());
	}

	// getoptassign is used to handle conversions from string

	template<typename T>
	void getoptassign(T t, const std::string& s)
	{
		std::stringstream ss(s);
		ss >> *t;
		if(!ss || !ss.eof())
			throwConversionException<T>(s);
	}

	template<>
	void getoptassign<std::string*>(std::string* t, const std::string& s)
	{
		*t = s;
	}

	template<>
	void getoptassign<bool*>(bool* t, const std::string& s)
	{
		auto scopy = s;
		transform(scopy.begin(), scopy.end(), scopy.begin(), ::tolower);
		if(scopy != "true" && scopy != "false")
			throwConversionException<bool>(scopy);
		*t = (scopy == "true");
	}

	enum class FlagType
	{
		// All non-solitary flags have content associated with them
		NONE,
		SHORT_SOLITARY,
		SHORT,// Could be a bundled set of shorts
		LONG_SOLITARY,
		LONG,
	};

	// Mutates @arg to the matched opt string
	// If content is found, replaces @optContent
	FlagType flagType(std::string& arg, std::string& optContent, GetOptConfiguration& config)
	{
		FlagType type = FlagType::NONE;
		auto argSize = arg.size();
		if(argSize >= 2 && arg[0] == '-')// Must be an opt
		{
			if(arg[1] == '-')// Long opt
			{
				auto equalsPos = arg.find('=');
				auto foundEquals = (equalsPos != std::string::npos);
				auto opt = arg.substr(2, (foundEquals
											? equalsPos - 2
											: std::string::npos));
				if(equalsPos != std::string::npos)
				{
					type = FlagType::LONG;
					optContent = arg.substr(equalsPos + 1);
				}
				else
					type = FlagType::LONG_SOLITARY;
				arg = std::move(opt);
			}
			else// Short opt
			{
				if(argSize != 2)// "-oSTUFF" or bundled shorts
				{
					// Non-bundling: assume the rest as the opt input
					//TODO Bundling branch
					type = FlagType::SHORT;
					optContent = arg.substr(2);
				}
				else// Otherwise we have ["-o", "STUFF"]
					type = FlagType::SHORT_SOLITARY;
				arg = std::move(arg[1]);
			}
		}
		return type;
	}

	// SolitaryOptHandle is used to handle types where an opt by itself makes sense.
	// This is either bool or an incremental ('+' at end) opt (which should be an int).

	template<class T, class Enable = void>
	class SolitaryOptHandle
	{
	public:
		static bool handle(T t, bool isIncremental)
		{
			return isIncremental;// This is behavior from D's getopt -- '+' at the end of non-integral types seems undefined and "handles" the opt
		}
	};
	 
	template<class T>
	class SolitaryOptHandle<T, typename std::enable_if<std::is_integral<T>::value >::type>
	{
	public:
		static bool handle(T t, bool isIncremental)
		{
			if(isIncremental)
				++*t;
			return isIncremental;
		}
	};

	template<>
	class SolitaryOptHandle<bool*, typename std::enable_if<true>::type>
	{
	public:
		static bool handle(bool* t, bool isIncremental)
		{
			*t = true;
			return true;
		}
	};

	using ArgVector = std::vector<std::string>;

	// TODO: Move semantics for option?
	// TODO: Could we include these all under the same struct template?

	template<typename T, typename...Ts>
	void getopthelper(ArgVector& args, decltype(args.size())& argsLimit
						, GetOptConfiguration& config, GetOptResult& result
						, const std::string& optSpec, T t, Ts&&...ts)
	{
		getopthelper(args, argsLimit, config, result, Option(optSpec), t, ts...);
	}

	template<typename T, typename...Ts>
	void getopthelper(ArgVector& args, decltype(args.size())& argsLimit
						, GetOptConfiguration& config, GetOptResult& result
						, const char* optSpec, T t, Ts&&...ts)
	{
		getopthelper(args, argsLimit, config, result, std::string(optSpec), t, ts...);
	}

	template<typename T, typename...Ts>
	void getopthelper(ArgVector& args, decltype(args.size())& argsLimit
						, GetOptConfiguration& config, GetOptResult& result
						, const std::string& optSpec, const std::string& help, T t, Ts&&...ts)
	{
		getopthelper(args, argsLimit, config, result, Option(optSpec, help), t, ts...);
	}

	template<typename T, typename...Ts>
	void getopthelper(ArgVector& args, decltype(args.size())& argsLimit
						, GetOptConfiguration& config, GetOptResult& result
						, const char* optSpec, const std::string& help, T t, Ts&&...ts)
	{
		getopthelper(args, argsLimit, config, result, std::string(optSpec), help, t, ts...);
	}

	template<typename T, typename...Ts>
	void getopthelper(ArgVector& args, decltype(args.size())& argsLimit
						, GetOptConfiguration& config, GetOptResult& result
						, const std::string& optSpec, const char* help, T t, Ts&&...ts)
	{
		getopthelper(args, argsLimit, config, result, optSpec, std::string(help), t, ts...);
	}

	template<typename T, typename...Ts>
	void getopthelper(ArgVector& args, decltype(args.size())& argsLimit
						, GetOptConfiguration& config, GetOptResult& result
						, const char* optSpec, const char* help, T t, Ts&&...ts)
	{
		getopthelper(args, argsLimit, config, result, std::string(optSpec), std::string(help), t, ts...);
	}

	template<typename T, typename...Ts>
	void getopthelper(ArgVector& args, decltype(args.size())& argsLimit
						, GetOptConfiguration& config, GetOptResult& result
						, const Option& option, T t, Ts&&...ts)
	{
		// Construct the opt
		bool foundFlag = false;
		if(result.parsing)// XXX: Does D return a complete Option[] after stopping?
		{
			std::string optContent;
			for(size_t argsIndex = 0; argsIndex < argsLimit;)// Flag find loop
			{
				std::string arg = args[argsIndex];
				if(!config.caseSensitive)
					std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);

				bool foundHere = false;
				bool expectingContentNext = false;
				auto type = flagType(arg, optContent, config);
				switch(type)// Remember: arg and optContent are mutated!
				{
					case FlagType::NONE:
						if(config.stopOnFirstNonOption)
						{
							result.parsing = false;
							goto END_PARSE;
						}
						break;
					case FlagType::SHORT_SOLITARY:
					case FlagType::LONG_SOLITARY:
						expectingContentNext = true;
					case FlagType::LONG:
					case FlagType::SHORT:
						switch(type)
						{
							case FlagType::LONG:
							case FlagType::LONG_SOLITARY:
								foundHere = foundFlag = (option.longOpts.count(arg) > 0);
								break;
							case FlagType::SHORT:
							case FlagType::SHORT_SOLITARY:
								foundHere = foundFlag = (option.shortOpts.count(arg[0]) > 0);
								break;
							default:
								throw std::logic_error("internal error: incorrect flag type after parse");
						}
						break;
					default:
						throw std::logic_error("Uncaught flag type");
				}
				// Handle find result and shift; we'll handle the args index shift ourselves
				if(foundHere)
				{
					auto shiftArgs = [&]()
					{
						args.erase(args.begin() + argsIndex);
						--argsLimit;
					};
					shiftArgs();
					if(expectingContentNext)// Haven't gotten the content yet
					{
						expectingContentNext = false;
						if(SolitaryOptHandle<T>::handle(t, option.isIncremental))
							continue;
						else
						{
							if(argsLimit <= argsIndex)
								throw GetOptException("Expected input after option " + arg);
							optContent = args[argsIndex];
							shiftArgs();
						}
					}
					getoptassign(t, optContent);
					continue;
				}
				else
					++argsIndex;// Do the shift, since we took nothing
			}
		}
		END_PARSE:
		if(!foundFlag && config.required)
			throw GetOptException("Required option " + option.spec + " was not supplied");
		result.options.push_back(option);
		getopthelper(args, argsLimit, config, result, ts...);
	}
	
	// config value
	template<typename...Ts>
	void getopthelper(ArgVector& args, decltype(args.size())& argsLimit
						, GetOptConfiguration& configuration, GetOptResult& result
						, config configOption, Ts&&...ts)
	{
		configuration.set(configOption);
		getopthelper(args, argsLimit, configuration, result, ts...);
	}

	void getopthelper(ArgVector& args, decltype(args.size())& argsLimit
						, GetOptConfiguration& config, GetOptResult& result)
	{
		// Baked-in help check
		if(!result.definedOption("help") && !result.definedOption('h'))
			getopthelper(args, argsLimit, config, result, "help|h", "Shows this help", &result.helpWanted);

		if(!config.passThrough) // Check for args we didn't get
		{
			for(auto i = 0; i < argsLimit; ++i)
			{
				std::string arg = args[i], content;
				switch(flagType(arg, content, config))
				{
					case FlagType::NONE:
						break;
					default:
						throw GetOptException("Unrecognized option " + args[i]);
				}
			}
		}
	}

	bool findTerminatorIndex(ArgVector& args, decltype(args.size())& argsLimit)//XXX: Inline?
	{
		for(auto i = 0; i < args.size(); ++i)
			if(args[i] == "--")
			{
				argsLimit = i;
				return true;
			}
		return false;
	}

	// These are almost certainly the functions you want as an end user.

	void defaultGetoptPrinter(std::ostream& os, const std::string& message, std::vector<Option> options)
	{
  		os << message << std::endl;
		size_t longestLong = 0;
		for(auto& o : options)
			for(auto& longOpt : o.longOpts)
				if(longOpt.size() > longestLong)
					longestLong = longOpt.size();
		for(auto& o : options)
			os << std::setw(2) << (o.shortOptForHelp.empty() ? " " : '-' + o.shortOptForHelp)
		  	   << std::setw(longestLong + 3) << (o.longOptForHelp.empty() ? " " : "--" + o.longOptForHelp)
		  	   << " " << o.help << std::endl;
	}

	void defaultGetoptPrinter(const std::string& message, std::vector<Option> options)
	{
		defaultGetoptPrinter(std::cout, message, options);
	}

	template<typename...Args>
	GetOptResult getopt(ArgVector& args, Args&&...getoptargs)
	{
		GetOptResult result;
		GetOptConfiguration config;
		auto argsLimit = args.size();
		bool hasTerminator = findTerminatorIndex(args, argsLimit);
		getopthelper(args, argsLimit, config, result, getoptargs...);
		if(hasTerminator && !config.keepEndOfOptions)
			args.erase(args.begin() + argsLimit);
		return result;
	}

	// C++-friendly interface

	struct GetOptResultAndArgs
	{
		GetOptResult result;
		ArgVector args;

		GetOptResultAndArgs() = default;
		GetOptResultAndArgs(GetOptResult r, ArgVector as) : result(r), args(as) {}
	};

	template<typename...Args>
	GetOptResultAndArgs getopt(int argc, char** argv, Args&&...getoptargs)
	{
		ArgVector args(argc);
		for(auto i = 0; i < argc; ++i)
			args[i] = argv[i];
		auto result = getopt(args, getoptargs...);
		return GetOptResultAndArgs(result, std::move(args));
	}
};

#endif