/**
 * Inspire form cxxopts(https://github.com/jarro2783/cxxopts)(MIT License)
 *
 * If there is no legal license conflict, the file will be distributed under the [see root project license] license.
 */

#ifndef GAL_TOOLBOX_COMMAND_LINE_PARSER_HPP
#define GAL_TOOLBOX_COMMAND_LINE_PARSER_HPP

#if !defined(GAL_COMMAND_LINE_PARSER_DELIMITER)
#define GAL_COMMAND_LINE_PARSER_DELIMITER ','
#endif

#include <string>
#include <string_view>
#include <exception>
#include <memory>
#include <regex>
#include <limits>
#include <sstream>

namespace gal::toolbox::parser
{
	namespace detail {
		using string	  = std::string;
		using string_view = std::string_view;

#if defined(_WIN32)
#define GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what) ("\'" + (what) + "\'")
#else
#define GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what) ("¡®" + (what) + "¡¯")
#endif

	}// namespace detail

	class Token : public std::enable_shared_from_this<Token> {
	public:
		using share_type												= std::shared_ptr<Token>;
		using string_type												= detail::string;
		using string_view_type											= detail::string_view;

		virtual share_type		 clone() const							= 0;

		virtual void			 parse(string_view_type text) const		= 0;

		virtual void			 parse() const							= 0;

		virtual bool			 has_default_value() const				= 0;

		virtual bool			 is_container() const					= 0;

		virtual bool			 has_implicit_cast() const				= 0;

		virtual string_type		 get_default_value() const				= 0;

		virtual string_view_type get_default_value_view() const			= 0;

		virtual string_type		 get_implicit_value() const				= 0;

		virtual string_view_type get_implicit_value_view() const		= 0;

		virtual share_type		 default_value(string_view_type value)	= 0;

		virtual share_type		 implicit_value(string_view_type value) = 0;

		virtual bool			 is_boolean() const						= 0;
	};

	class ParserException : public std::exception {
	public:
		using string_type = detail::string;

		explicit ParserException(string_type message) : message(std::move(message)) {}

		explicit ParserException(const char* message) : message(message) {}

		[[nodiscard]] const char* what() const noexcept override {
			return message.c_str();
		}

	private:
		string_type message;
	};

	class SpecifyArgException : public ParserException {
	public:
		using ParserException::ParserException;
	};

	class ParseArgException : public ParserException {
	public:
		using ParserException::ParserException;
	};

	class ArgDuplicateException : public SpecifyArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgDuplicateException(const string_type& what)
			: SpecifyArgException("Arg already exists: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what)) {}
	};

	class ArgInvalidException : public SpecifyArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgInvalidException(const string_type& what)
			: SpecifyArgException("Arg has invalid format: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what)) {}
	};

	class ArgSyntaxException : public ParseArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgSyntaxException(const string_type& what)
			: ParseArgException("Arg has incorrect syntax: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what)) {}
	};

	class ArgFakeException : public ParseArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgFakeException(const string_type& what)
			: ParseArgException("Arg does not exists: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what)) {}
	};

	class ArgMissingException : public ParseArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgMissingException(const string_type& what)
			: ParseArgException("Arg is missing an argument: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what)) {}
	};

	class ArgNotSatisfiedException : public ParseArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgNotSatisfiedException(const string_type& what)
			: ParseArgException("Arg is not satisfied: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what)) {}
	};

	class ArgRejectException : public ParseArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgRejectException(const string_type& what, const string_type& given)
			: ParseArgException("Arg is reject to give: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what) +
								"\n\tbut still given: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(given)) {}
	};

	class ArgNotPresentException : public ParseArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgNotPresentException(const string_type& what)
			: ParseArgException("Arg not present: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what)) {}
	};

	class ArgEmptyException : public ParseArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgEmptyException(const string_type& what)
			: ParseArgException("Arg is empty: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what)) {}
	};

	class ArgBadTypeException : public ParseArgException {
	public:
		using string_type = typename SpecifyArgException::string_type;

		explicit ArgBadTypeException(const string_type& what)
			: ParseArgException("Arg has a bad type and failed to parse: " + GAL_COMMAND_LINE_PARSER_ARG_WRAPPER(what)) {}
	};

	namespace detail {
		constexpr const char* integer_pattern = "(-)?(0x)?([0-9a-zA-Z]+)|((0x)?0)";
		constexpr const char* truthy_pattern  = "(t|T)(rue)?|1";
		constexpr const char* falsy_pattern	  = "(f|F)(alse)?|0";
		constexpr const char* arg_specifier_pattern	  = "(([[:alnum:]]),)?[ ]*([[:alnum:]][-_[:alnum:]]*)?";
		constexpr const char* arg_matcher_pattern	  = "--([[:alnum:]][-_[:alnum:]]+)(=(.*))?|-([[:alnum:]]+)";

		struct IntegerDescriptor {
			using string_type = detail::string;

			string_type negative;
			string_type base;
			string_type value;
		};

		struct ArgumentDescriptor
		{
			using string_type = detail::string;

			string_type arg_name;
			bool grouping;
			bool set_value;
			string_type value;
		};

		IntegerDescriptor split_integer(const string& text)
		{
			const static std::basic_regex<typename string::value_type> integer_regex{integer_pattern};

			std::match_results<typename string::const_iterator> match;
			std::regex_match(text, match, integer_regex);

			if (match.length() == 0)
			{
				throw ArgBadTypeException{text};
			}

			IntegerDescriptor ret{match[1]};

			if (match.length(4) > 0)
			{
				ret.base = match[5];
				ret.value = "0";
			}
			else
			{
				ret.base = match[2];
				ret.value = match[3];
			}

			return ret;
		}

		bool is_true_text(const string& text)
		{
			const static std::basic_regex<typename string::value_type> truthy_regex{truthy_pattern};

			std::match_results<typename string::const_iterator> match;
			std::regex_match(text, match, truthy_regex);

			return !match.empty();
		}

		bool is_false_text(const string& text)
		{
			const static std::basic_regex<typename string::value_type> falsy_regex{truthy_pattern};

			std::match_results<typename string::const_iterator> match;
			std::regex_match(text, match, falsy_regex);

			return !match.empty();
		}

		std::pair<string, string> get_short_and_long_format_arg(const string& text)
		{
			const static std::basic_regex<typename string::value_type> specifier_regex{arg_specifier_pattern};

			std::match_results<const char*> result;
			std::regex_match(text.c_str(), result, specifier_regex);

			if (result.empty())
			{
				throw ArgInvalidException{text};
			}

			return std::make_pair(result[2], result[3]);
		}

		bool parse_argument(const char* arg, ArgumentDescriptor& desc)
		{
			const static std::basic_regex<char> matcher_regex{arg_matcher_pattern};

			std::match_results<const char*> result;
			std::regex_match(arg, result, matcher_regex);

			bool matched = !result.empty();

			if (matched)
			{
				if (result[4].length() > 0)
				{
					desc.arg_name = result[4].str();
					desc.grouping = true;
				}
				else
				{
					desc.arg_name = result[1].str();
					desc.grouping = false;
				}
				desc.set_value = result[2].length() > 0;
				desc.value = result[3].str();
			}

			return matched;
		}

		template<typename T, typename U>
		void range_checker(U num, bool negative, const string& text)
		{
			if constexpr (std::numeric_limits<T>::is_signed)
			{
				if (negative)
				{
					if (num > static_cast<U>(std::numeric_limits<T>::min())
					{
						throw ArgBadTypeException{text};
					}
				}
				else
				{
					if (num > static_cast<U>(std::numeric_limits<T>::max()))
					{
						throw ArgBadTypeException{text};
					}
				}
			}
			else
			{
				return;
			}
		}

		template<typename R, typename T>
		void negate_check_and_set(R& r, T&& t, const string& text)
		{
			if constexpr (std::numeric_limits<R>::is_signed)
			{
				r = static_cast<R>(-static_cast<R>(t - 1) - 1);
			}
			else
			{
				throw ArgBadTypeException{text};
			}
		}

		template<typename T>
		void integral_parser(const string& text, T& value)
		{
			static_assert(std::is_integral_v<T>, "not supportted type");

			if constexpr (std::is_same_v<T, bool>)
			{
				if (is_true_text(text))
				{
					value = true;
				}
				else if(is_false_text(text))
				{
					value = false;
				}
				else
				{
					throw ArgBadTypeException{text};
				}
			}
			else if constexpr (std::is_same_v<T, char>)
			{
				if (text.length() != 1)
				{
					throw ArgBadTypeException{text};
				}

				value = text[0];
			}
			else
			{
				using unsigned_type = std::make_unsigned_t<T>;

				const bool is_neg = !desc.negative.empty();
				const auto desc = split_integer(text);

				unsigned_type result = 0;

				for (bool is_hex = !desc.base.empty(); char c : desc.value)
				{
					unsigned_type digit;
				
					if (c >= '0' && c <= '9')
					{
						digit = static_cast<unsigned_type>(c - '0');
					}
					else if (is_hex && c >= 'a' && c <= 'f')
					{
						digit = static_cast<unsigned_type>(c - 'a' + 10);
					}
					else if (is_hex && c >= 'A' && c <= 'F')
					{
						digit = static_cast<unsigned_type>(c - 'A' + 10);
					}
					else
					{
						throw ArgBadTypeException{text};
					}

					auto next = static_cast<unsigned_type>(result * (is_hex ? 16 : 10) + digit);
					if (result > next)
					{
						throw ArgBadTypeException{text};
					}

					result = next;
				}

				range_checker<T>(result, is_neg, text);

				if (is_neg)
				{
					negate_check_and_set(value, result, text);
				}
				else
				{
					value = static_cast<T>(result);
				}
			}
		}

		template<typename T>
		void stringstream_parser(const string& text, T& value)
		{
			std::basic_stringstream<typename string::value_type> in(text);
			in >> value;
			if (!in)
			{
				throw ArgBadTypeException{text};
			}
		}

		template<typename T>
		void token_parser(const string& text, T& value)
		{
			stringstream_parser(text, value);
		}

		#if __has_include(<vector>)
			template<typename T>
			void token_parser(const string& text, std::vector<T>& value)
			{
				std::basic_stringstream<typename string::value_type> in(text);

				string t;
				while (!in.eof() && std::getline(in, t, GAL_COMMAND_LINE_PARSER_DELIMITER))
				{
					T curr;
					token_parser(t, curr);
					value.emplace_back(std::move(curr));
				}
			}
		#endif

		#if __has_include(<optional>)
			template<typename T>
			void token_parser(const string& text, std::optional<T>& value)
			{
				T result;
				token_parser(text, result);
				value = std:::move(result);
			}
		#endif
	}// namespace detail

	

}// namespace gal::toolbox::parser

#endif//GAL_TOOLBOX_COMMAND_LINE_PARSER_HPP
