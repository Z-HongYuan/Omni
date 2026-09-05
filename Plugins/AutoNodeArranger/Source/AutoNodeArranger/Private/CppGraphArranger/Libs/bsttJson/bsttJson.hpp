#pragma once

#include <charconv>
#include <cstdint>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

inline std::string to_string(const std::string& s) { return s; }

#ifdef USE_BSTT_NAMESPACE
namespace bstt
{
#endif
	static constexpr size_t MAX_JSON_DEPTH = 1024;

	static const std::string& getTab(const std::string& tab, size_t tabCount)
	{
		static std::map<std::string, std::vector<std::string>> tabListMap;
		static std::string emptyTab;
		if (tab.empty()) return emptyTab;
		auto it = tabListMap.find(tab);
		if (it == tabListMap.end())
		{
			tabListMap.emplace(tab, std::vector<std::string>{"", tab});
			it = tabListMap.find(tab);
			it->second.reserve(MAX_JSON_DEPTH);
		}
		while (it->second.size() <= tabCount) it->second.push_back(it->second.back() + tab);
		return it->second[tabCount];
	}

	using JsonObj = std::map<std::string, struct Json>;
	using JsonArr = std::vector<struct Json>;

	template <typename T> T from_string(const std::string& s);

	template <> inline std::string from_string<std::string>(const std::string& s) { return s; }
	template <> inline int from_string<int>(const std::string& s) { return std::stoi(s); }
	template <> inline int64_t from_string<int64_t>(const std::string& s) { return std::stoll(s); }
	template <> inline size_t from_string<size_t>(const std::string& s) { return std::stoull(s); }
	template <> inline double from_string<double>(const std::string& s) { return std::stod(s); }

	template <typename T> T fromJson(const Json&);

	template <typename T> Json toJson(const T&);

	void parseValue(const std::string_view& str, size_t& pos, Json& jsonValue, size_t depth);

	struct Json
	{
		enum class Type
		{
			Null,
			Bool,
			Number,
			String,
			Array,
			Object
		};

		static std::string typeToString(Type type)
		{
			static const std::vector<std::string> typeStringArr = {"Null", "Bool", "Number", "String", "Array", "Object"};
			return typeStringArr[static_cast<size_t>(type)];
		}

		template <typename T> static Json::Type typeToType(const T&) { return Type::Null; }
		static Type typeToType(const bool&) { return Type::Bool; }
		static Type typeToType(const int&) { return Type::Number; }
		static Type typeToType(const int64_t&) { return Type::Number; }
		static Type typeToType(const size_t&) { return Type::Number; }
		static Type typeToType(const double&) { return Type::Number; }
		static Type typeToType(const std::string&) { return Type::String; }
		static Type typeToType(const JsonObj&) { return Type::Object; }
		static Type typeToType(const JsonArr&) { return Type::Array; }

		static Json parse(const std::string_view& str)
		{
			Json json;
			size_t pos = 0;
			parseValue(str, pos, json, 0);
			if (pos != str.size()) throw std::runtime_error("Extra characters at position " + std::to_string(pos));
			return json;
		}

		static bool tryParse(const std::string_view& str, Json& json)
		{
			std::string error;
			return tryParse(str, json, error);
		}

		static bool tryParse(const std::string_view& str, Json& json, std::string& error)
		{
			size_t pos = 0;
			try
			{
				parseValue(str, pos, json, 0);
				if (pos != str.size()) throw std::runtime_error("Extra characters at position " + std::to_string(pos));
				return true;
			}
			catch (const std::exception& e)
			{
				error = e.what();
				return false;
			}
		}

		static Json parseFile(const std::string& fileName)
		{
			std::ifstream ifs(fileName);
			std::ostringstream oss;
			oss << ifs.rdbuf();
			return parse(oss.str());
		}

		static bool tryParseFile(const std::string& fileName, Json& json)
		{
			std::string error;
			return tryParseFile(fileName, json, error);
		}

		static bool tryParseFile(const std::string& fileName, Json& json, std::string& error)
		{
			std::ifstream ifs(fileName);
			std::ostringstream oss;
			oss << ifs.rdbuf();
			return tryParse(oss.str(), json, error);
		}

		// Constructors

		Json() : b(false) {} // default is null
		Json(const Json& v) { *this = v; }
		template <typename T> Json(const T& v) { *this = v; }

		// Move constructor

		Json(Json&& v) noexcept : type(v.type), b(false)
		{
			switch (v.type)
			{
			case Type::Null:
				break;
			case Type::Bool:
				b = v.b;
				break;
			case Type::Number:
				num = v.num;
				break;
			case Type::String:
				new (&str) std::string(std::move(v.str));
				break;
			case Type::Object:
				new (&obj) JsonObj(std::move(v.obj));
				break;
			case Type::Array:
				new (&arr) JsonArr(std::move(v.arr));
				break;
			}
			v.type = Type::Null;
		}

		// Destructor

		~Json() {}

		// Assignments

		Json& operator=(const Json& rhs)
		{
			if (this == &rhs) return *this;
			type = rhs.type;
			switch (rhs.type)
			{
			case Type::Null:
				b = false;
				break;
			case Type::Bool:
				b = rhs.b;
				break;
			case Type::Number:
				num = rhs.num;
				break;
			case Type::String:
				*this = rhs.str;
				break;
			case Type::Object:
				*this = rhs.obj;
				break;
			case Type::Array:
				*this = rhs.arr;
				break;
			}
			return *this;
		}
		Json& operator=(std::nullptr_t)
		{
			type = Type::Null;
			return *this;
		}
		Json& operator=(bool b_)
		{
			type = Type::Bool;
			b = b_;
			return *this;
		}
		Json& operator=(int i)
		{
			type = Type::Number;
			num = i;
			return *this;
		}
		Json& operator=(int64_t i)
		{
			type = Type::Number;
			num = static_cast<double>(i);
			return *this;
		}
		Json& operator=(size_t i)
		{
			type = Type::Number;
			num = static_cast<double>(i);
			return *this;
		}
		Json& operator=(double d_)
		{
			type = Type::Number;
			num = d_;
			return *this;
		}
		Json& operator=(const char* s_)
		{
			type = Type::String;
			new (&str) std::string(s_);
			// replace tabs and newlines with escape sequences
			size_t pos = 0;
			while ((pos = str.find_first_of("\t\r\n", pos)) != std::string::npos)
			{
				if (str[pos] == '\t') str.replace(pos, 1, "\\t");
				else if (str[pos] == '\r')
					str.replace(pos, 1, "\\r");
				else
					str.replace(pos, 1, "\\n");
				pos += 2;
			}
			return *this;
		}
		Json& operator=(const std::string& s_) { return *this = s_.c_str(); }
		Json& operator=(const JsonObj& obj_)
		{
			type = Type::Object;
			new (&obj) JsonObj(obj_);
			return *this;
		}
		Json& operator=(const JsonArr& arr_)
		{
			type = Type::Array;
			new (&arr) JsonArr(arr_);
			return *this;
		}
		template <typename T> Json& operator=(const T& t)
		{
			*this = toJson<T>(t);
			return *this;
		}
		template <typename T, typename U> Json& operator=(const std::map<T, U>& tuMap)
		{
			using namespace std;
			type = Type::Object;
			new (&obj) JsonObj();
			for (const auto& [key, value] : tuMap) (*this)[to_string(key)] = value;
			return *this;
		}
		template <typename T> Json& operator=(const std::vector<T>& tList)
		{
			type = Type::Array;
			new (&arr) JsonArr();
			arr.resize(tList.size());
			for (size_t i = 0; i < tList.size(); i++) (*this)[i] = tList[i];
			return *this;
		}

		// Move assignment

		Json& operator=(Json&& rhs) noexcept
		{
			if (this == &rhs) return *this;
			type = rhs.type;
			switch (rhs.type)
			{
			case Type::Null:
				break;
			case Type::Bool:
				b = rhs.b;
				break;
			case Type::Number:
				num = rhs.num;
				break;
			case Type::String:
				new (&str) std::string(std::move(rhs.str));
				break;
			case Type::Object:
				new (&obj) JsonObj(std::move(rhs.obj));
				break;
			case Type::Array:
				new (&arr) JsonArr(std::move(rhs.arr));
				break;
			}
			rhs.type = Type::Null;
			return *this;
		}

		// Conversions

		operator const bool&() const { return b; }
		operator int() const { return static_cast<int>(num); }
		operator int64_t() const { return static_cast<int64_t>(num); }
		operator size_t() const { return static_cast<size_t>(num); }
		operator const double&() const { return num; }
		operator std::string() const { return str; }
		operator const char*() const { return str.c_str(); }
		operator const JsonObj&() const { return obj; }
		operator const JsonArr&() const { return arr; }
		template <typename T> operator T() const { return fromJson<T>(*this); }
		template <typename T, typename U> operator std::map<T, U>() const
		{
			std::map<T, U> tuMap;
			for (const auto& [key_, val_] : obj) tuMap[key_] = val_;
			return tuMap;
		}
		template <typename T> operator std::vector<T>() const
		{
			std::vector<T> tList;
			for (const auto& value : arr) tList.push_back(fromJson<T>(value));
			return tList;
		}

		// Get

		template <typename T> void get(const std::string& key, T& value) const
		{
			const auto& child = (*this)[key];
			child.checkKeyType(key, typeToType(value));
			value = child;
		}
		void get(const std::string& key, std::string& value) const
		{
			const auto& child = (*this)[key];
			child.checkKeyType(key, Type::String);
			value = std::string(child.str);
		}
		template <typename T, typename U> void get(const std::string& key, std::map<T, U>& value) const
		{
			const auto& child = (*this)[key];
			child.checkKeyType(key, Type::Object);
			for (const auto& [key_, val_] : child.obj) value[from_string<T>(key_)] = val_;
		}
		template <typename T> void get(const std::string& key, std::vector<T>& value) const
		{
			const auto& child = (*this)[key];
			child.checkKeyType(key, Type::Array);
			value = child.operator std::vector<T, std::allocator<T>>();
		}
		template <typename T, typename... Args> void get(const std::string& key, T& value, Args&&... args) const
		{
			get(key, value);
			get(args...);
		}

		// Set

		template <typename T> void set(const std::string& key, const T& value) { (*this)[key] = value; }
		template <typename T, typename... Args> void set(const std::string& key, const T& value, Args&&... args)
		{
			set(key, value);
			set(args...);
		}

		// Try get

		template <typename T> bool tryGet(const std::string& key, T& value) const
		{
			auto it = obj.find(key);
			if (it != obj.end()) value = it->second;
			return it != obj.end();
		}
		bool tryGet(const std::string& key, std::string& value) const
		{
			auto it = obj.find(key);
			if (it != obj.end())
			{
				it->second.checkKeyType(key, Type::String);
				value = std::string(it->second.str);
			}
			return it != obj.end();
		}
		template <typename T, typename U> bool tryGet(const std::string& key, std::map<T, U>& value) const
		{
			auto it = obj.find(key);
			if (it != obj.end())
			{
				it->second.checkKeyType(key, Type::Object);
				for (const auto& [k, val] : it->second.obj) value[from_string<T>(k)] = val;
			}
			return it != obj.end();
		}
		template <typename T> bool tryGet(const std::string& key, std::vector<T>& value) const
		{
			auto it = obj.find(key);
			if (it != obj.end())
			{
				it->second.checkKeyType(key, Type::Array);
				value = std::vector<T>(it->second);
			}
			return it != obj.end();
		}
		template <typename T, typename... Args> bool tryGet(const std::string& key, T& value, Args&&... args) const
		{
			bool allFound = tryGet(key, value);
			return tryGet(args...) && allFound;
		}

		// Array functions

		const Json& operator[](size_t index) const { return arr[index]; }
		Json& operator[](size_t index)
		{
			if (type != Type::Array) *this = JsonArr();
			if (arr.size() <= index) arr.resize(index + 1);
			return arr[index];
		}

		void resize(size_t size) { arr.resize(size); }

		// Object functions

		const Json& operator[](const std::string& key) const
		{
			auto it = obj.find(key);
			if (it == obj.end()) throw std::runtime_error("Key not found: '" + key + "'");
			return it->second;
		}
		const Json& operator[](const char* key) const { return (*this)[std::string(key)]; }

		Json& operator[](const std::string& key)
		{
			if (type != Type::Object) *this = JsonObj();
			return obj[key];
		}
		Json& operator[](const char* key)
		{
			if (type != Type::Object) *this = JsonObj();
			return obj[key];
		}

		// Display

		std::string toString(const std::string& tab = "", const std::string& newLine = "") const
		{
			std::ostringstream ostr;
			display(ostr, tab, newLine);
			return ostr.str();
		}

		friend std::ostream& operator<<(std::ostream& os, const Json& v) { return v.display(os); }

		std::ostream& display(
			std::ostream& os, const std::string& tab = "", const std::string& newLine = "", size_t currentTabCount = 0) const
		{
			switch (type)
			{
			case Type::Null:
				return os << "null";
			case Type::Bool:
				return os << (b ? "true" : "false");
			case Type::Number:
				return os << num;
			case Type::String:
				return os << '"' << str << '"';
			case Type::Object:
				return displayAsObject(os, currentTabCount, tab, newLine);
			case Type::Array:
				return displayAsArray(os, currentTabCount, tab, newLine);
			}
			return os;
		}

		void writeToFile(const std::string& fileName, const std::string& tab = "", const std::string& newLine = "") const
		{
			std::ofstream ofs(fileName);
			ofs << toString(tab, newLine);
		}

		// Getters

		Type getType() const { return type; }

	private:
		Type type = Type::Null;

		union
		{
			std::string str;
			bool b;
			double num;
			JsonArr arr;
			JsonObj obj; // TODO: add a map key to index, and change JsonObj to a map of index to Pair<string, Json>
		};

		void get() const {}
		static bool tryGet() { return true; }
		void set() {}

		void checkKeyType(const std::string& key, Type expectedType) const
		{
			if (expectedType == Type::Null) return; // allow any type
			if (type != expectedType)
				throw std::runtime_error(
					"Expected " + typeToString(expectedType) + " but got " + typeToString(type) + " for key '" + key + "'");
		}

		std::ostream& displayAsObject(
			std::ostream& os, size_t currentTabCount, const std::string& tab, const std::string& newLine) const
		{
			if (obj.empty()) return os << "{}";
			size_t newTabCount = currentTabCount + 1;
			const auto& newTab = getTab(tab, newTabCount);
			auto it = obj.begin();
			auto beforeEnd = obj.size() - 1;
			os << "{";
			for (size_t i = 0; i < beforeEnd; ++i, ++it)
				it->second.display(os << newLine << newTab << '"' << it->first << "\": ", tab, newLine, newTabCount) << ", ";
			return it->second.display(os << newLine << newTab << '"' << it->first << "\": ", tab, newLine, newTabCount)
				   << newLine << getTab(tab, currentTabCount) << "}";
		}

		std::ostream& displayAsArray(
			std::ostream& os, size_t currentTabCount, const std::string& tab, const std::string& newLine) const
		{
			if (arr.empty()) return os << "[]";
			size_t newTabCount = currentTabCount + 1;
			const auto& newTab = getTab(tab, newTabCount);
			auto it = arr.begin();
			auto beforeEnd = arr.size() - 1;
			os << "[";
			for (size_t i = 0; i < beforeEnd; ++i, ++it) it->display(os << newLine << newTab, tab, newLine, newTabCount) << ", ";
			return it->display(os << newLine << newTab, tab, newLine, newTabCount)
				   << newLine << getTab(tab, currentTabCount) << "]";
		}
	};

	namespace detail
	{

		inline void skipSpace(const std::string_view& str, size_t& pos)
		{
			while (pos < str.size() && std::isspace(str[pos])) ++pos;
		}

		inline void parseChar(const std::string_view& str, size_t& pos, char c)
		{
			if (pos >= str.size() || str[pos] != c)
				throw std::runtime_error("Expected '" + std::string(1, c) + "' at position " + std::to_string(pos));
			++pos;
		}

		inline void parseHex(const std::string_view& str, size_t& pos)
		{
			if (!std::isxdigit(str[pos])) throw std::runtime_error("Expected hex digit at position " + std::to_string(pos));
			++pos;
		}

		inline void parseEscape(const std::string_view& str, size_t& pos)
		{
			switch (str[pos])
			{
			case '"':
			case '\\':
			case '/':
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't':
				++pos;
				break;
			case 'u':
				++pos;
				for (size_t i = 0; i < 4; ++i) parseHex(str, pos);
				break;
			default:
				throw std::runtime_error("Invalid escape character at position " + std::to_string(pos));
			}
		}

		inline void parseString(const std::string_view& str, size_t& pos, std::string& value)
		{
			size_t start = pos;
			while (str[pos] != '"' && pos < str.size())
			{
				if (str[pos] == '\\')
				{
					++pos;
					parseEscape(str, pos);
				}
				else if (str[pos] == '\n' || str[pos] == '\t' || str[pos] == '\r')
					throw std::runtime_error("Invalid character in string at position " + std::to_string(pos));
				else
					++pos;
			}
			value = str.substr(start, pos - start);
			parseChar(str, pos, '"');
		}

		inline void parseDigits(const std::string_view& str, size_t& pos)
		{
			if (!std::isdigit(str[pos])) throw std::runtime_error("Invalid number at position " + std::to_string(pos));
			while (std::isdigit(str[pos])) ++pos;
		}

		inline void parseExponent(const std::string_view& str, size_t& pos)
		{
			++pos;
			if (str[pos] == '+' || str[pos] == '-') ++pos;
			parseDigits(str, pos);
		}

		inline void parseDecimal(const std::string_view& str, size_t& pos)
		{
			++pos;
			parseDigits(str, pos);
			if (str[pos] == 'e' || str[pos] == 'E') parseExponent(str, pos);
		}

		inline void parseNumber(const std::string_view& str, size_t& pos, double& value)
		{
			size_t start = pos;
			if (str[pos] == '-') ++pos;
			if (str[pos] == '0')
			{
				++pos;
				if (str[pos] == '.') parseDecimal(str, pos);
				else if (str[pos] == 'e' || str[pos] == 'E')
					parseExponent(str, pos);
			}
			else
			{
				parseDigits(str, pos);
				if (str[pos] == '.') parseDecimal(str, pos);
				else if (str[pos] == 'e' || str[pos] == 'E')
					parseExponent(str, pos);
			}
#ifdef __cpp_lib_to_chars
			std::from_chars(str.data() + start, str.data() + pos, value);
#else
		value = std::stod(std::string(str.substr(start, pos - start)));
#endif
		}

		inline void parseObject(const std::string_view& str, size_t& pos, Json& jsonValue, size_t depth)
		{
			skipSpace(str, pos);
			if (str[pos] == '}') jsonValue = JsonObj();
			while (pos < str.size() && str[pos] != '}')
			{
				parseChar(str, pos, '"');
				std::string key;
				parseString(str, pos, key);
				skipSpace(str, pos);
				parseChar(str, pos, ':');
				Json value;
				parseValue(str, pos, value, depth + 1);
				jsonValue[key] = value;
				if (str[pos] == '}') break;
				parseChar(str, pos, ',');
				skipSpace(str, pos);
				if (str[pos] == '}') throw std::runtime_error("Extra comma at position " + std::to_string(pos));
			}
			parseChar(str, pos, '}');
		}

		inline void parseArray(const std::string_view& str, size_t& pos, Json& jsonValue, size_t depth)
		{
			size_t i = 0;
			if (str[pos] == ']') jsonValue = JsonArr();
			while (pos < str.size() && str[pos] != ']')
			{
				Json value;
				parseValue(str, pos, value, depth + 1);
				jsonValue[i++] = value;
				if (str[pos] == ']') break;
				parseChar(str, pos, ',');
				skipSpace(str, pos);
				if (str[pos] == ']') throw std::runtime_error("Extra comma at position " + std::to_string(pos));
			}
			parseChar(str, pos, ']');
		}

	} // namespace detail

	inline void parseValue(const std::string_view& str, size_t& pos, Json& jsonValue, size_t depth)
	{
		using namespace detail;

		if (depth == MAX_JSON_DEPTH) throw std::runtime_error("Exceeded maximum depth of " + std::to_string(MAX_JSON_DEPTH));

		skipSpace(str, pos);
		std::string s;
		double d = 0.0;
		switch (str[pos])
		{
		case 'n':
			pos++;
			parseChar(str, pos, 'u');
			parseChar(str, pos, 'l');
			parseChar(str, pos, 'l');
			break;
		case 't':
			pos++;
			jsonValue = true;
			parseChar(str, pos, 'r');
			parseChar(str, pos, 'u');
			parseChar(str, pos, 'e');
			break;
		case 'f':
			pos++;
			jsonValue = false;
			parseChar(str, pos, 'a');
			parseChar(str, pos, 'l');
			parseChar(str, pos, 's');
			parseChar(str, pos, 'e');
			break;
		case '"':
			pos++;
			parseString(str, pos, s);
			jsonValue = s;
			break;
		case '[':
			pos++;
			parseArray(str, pos, jsonValue, depth);
			break;
		case '{':
			pos++;
			parseObject(str, pos, jsonValue, depth);
			break;
		default:
			parseNumber(str, pos, d);
			jsonValue = d;
			break;
		}
		skipSpace(str, pos);
	}
#ifdef USE_BSTT_NAMESPACE
} // namespace bstt
#endif
