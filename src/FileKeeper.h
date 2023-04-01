#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <stack>
#include <sstream>

#define TOP_ASCII 95
#define CRYPT_LONG 128

namespace filek {
	class scMachine {
		private: static std::string password, curr_password;
		private: static size_t pointer_pw, SIZE_OF_PW, MAX_PAR_SIZE;
		private: static void abacusPassword(const unsigned int times) {
			std::string ordw;
			for (unsigned int i = 0; i < times; i++) {
				ordw = curr_password.substr(1, curr_password.size() - 1);
				ordw.append(1, curr_password.front());
				curr_password.replace(0, curr_password.length(), ordw);
			}
		}
		private: inline static void encriptKey(std::string& key) {
			int chara = 0;
			std::string crypto = "";
			for (int i = 0, j = key.size() - 1; i < key.size(); i++, j--) {
				chara = key.at((i % 2 == 1 ? i : j)) + key.at(i) - 64;
				chara %= TOP_ASCII;
				crypto.append(1, chara + 32);
			}
			key = crypto;
				
		}

		public:  static void encriptWord(std::string& str) {
			size_t chara = 0, textPointer = 0, keyPointer = 0, link = 0, pastLink = 1;
			std::string buffer;
			MAX_PAR_SIZE = (CRYPT_LONG < str.size() ? str.size() : CRYPT_LONG);
			for (size_t i = 0; i < MAX_PAR_SIZE; i++) {
				if (textPointer == str.size()) {
					chara = curr_password.at(keyPointer) + link;
					link = 32;
				} else {
					chara = str[textPointer] + curr_password.at(keyPointer) + link - 64;
					link = (str[textPointer] - 32);
					textPointer++;
				}
				chara %= TOP_ASCII;
				buffer.append(1, chara + 32);
				keyPointer = (keyPointer + pastLink) % SIZE_OF_PW;
				if (pointer_pw < SIZE_OF_PW) 
					pointer_pw++;
				else {
					abacusPassword(pastLink);
					pastLink = chara % SIZE_OF_PW;
					pointer_pw = 0;
				}
					
			}
			str = buffer;
		}
		public:	 static void decriptWord(std::string& str) {
			int keyPointer = 0, link = 0, pastLink = 1;
			int chara = 0;
			std::string unCrypto = "";
			for (unsigned int i = 0; i < str.size(); i++) {
				chara = str[i] - link - 32;
				if (chara < 0)
					chara += TOP_ASCII;
				chara -= curr_password.at(keyPointer) - 32;
				if (chara < 0)
					chara += TOP_ASCII;
				link = chara;
				unCrypto.append(1, chara + 32);
				keyPointer = (keyPointer + pastLink) % SIZE_OF_PW;
				if (pointer_pw < SIZE_OF_PW) 
					pointer_pw++;
				else {
					abacusPassword(pastLink);
					pastLink = (str[i] - 32) % SIZE_OF_PW;
					pointer_pw = 0;
				}
			}
			if (!unCrypto.empty())
				while (unCrypto.back() == 64) {
					unCrypto.pop_back();
					if (unCrypto.empty())
						break;
				}
			str = unCrypto;
		}
		public:  inline static void setPassword(const std::string& pwd) {
			password = pwd; 
			encriptKey(password);
			resetPassword(); 
			SIZE_OF_PW = pwd.size();
		}
		public:  inline static void resetPassword() {
			curr_password = password; pointer_pw = 0;
		}
	};
	size_t scMachine::pointer_pw = 0;
	size_t scMachine::MAX_PAR_SIZE = CRYPT_LONG;
	size_t scMachine::SIZE_OF_PW = 0;
	std::string scMachine::password = "";
	std::string scMachine::curr_password = "";

	class dynamicfile {
		private: std::vector<std::string> vec_info;
		private: std::vector<std::pair<std::string, dynamicfile>> inception;
		private: std::unordered_map<std::string, size_t> incepMap;

		public: inline dynamicfile& operator[](const std::string& name) {
			if (this->incepMap.count(name) == 0) {
				incepMap[name] = inception.size();
				inception.push_back({ name, dynamicfile() });
			}
			return inception[incepMap[name]].second;
		}
		public: static bool writeFile(const dynamicfile& df, const std::string& fileName, const bool encript = false, const char split = ',') {
			scMachine::resetPassword();
			std::string splitter = std::string(1, split) + " ", buffer = ""; size_t indexGrade = 0;
			std::function <void(const dynamicfile&, std::ofstream&)> write = [&](const dynamicfile& dF, std::ofstream& writter) {
				for (std::pair<std::string, dynamicfile> const& property : dF.inception)
					if (property.second.inception.empty()) {
						buffer.clear();
						buffer += property.first + " = ";
						size_t items = property.second.getValueCount();
						for (size_t i = 0; i < property.second.getValueCount(); i++) {
							size_t n = property.second.getString(i).find_first_of(split);
							if (n != std::string::npos) 
								buffer += "\"" + property.second.getString(i) + "\"" + (items > 1 ? splitter : "");
							else
								buffer += property.second.getString(i) + (items > 1 ? splitter : "");
							items--;
						}
						if (encript)
							scMachine::encriptWord(buffer);
						writter << (encript ? "" : std::string(indexGrade, '\t')) << buffer << "\n";
						buffer.clear();
					} else {
						buffer += property.first + " {";
						if (encript)
							scMachine::encriptWord(buffer);
						writter << (encript ? "" : std::string(indexGrade, '\t')) << buffer << "\n";
						indexGrade++;
						write(property.second, writter);
						buffer.clear();
						buffer += "}";
						if (encript)
							scMachine::encriptWord(buffer);
						writter << (encript ? "" : std::string(indexGrade, '\t')) << buffer << "\n";
					}
				if (indexGrade > 0) indexGrade--;
			};
			
			std::ofstream file(fileName, std::ios::out);
			if (file.fail())
				return false;
			write(df, file);
			file.close();
			return true;
		}
		public: static bool readFile(dynamicfile& df, const std::string& fileName, const bool decript = false, const char split = ',') {
			scMachine::resetPassword();
			std::ifstream file(fileName, std::ios::in);
			if (file.fail())
				return false;
			std::string propName = "", propValue = "", buffer = "", subString = "";
			size_t tkCount = 0;
			bool quotemode = false;
			std::stack<std::reference_wrapper<dynamicfile>> stackFile;
			stackFile.push(df);
			std::function <void(std::string&)> adjust = [](std::string& str) {
				str.erase(0, str.find_first_not_of(" \t\n\r\f\v"));
				str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
			};
			while (!file.eof()) {
				std::getline(file, buffer);
				if (decript)
					scMachine::decriptWord(buffer);
				adjust(buffer);
				if (!buffer.empty()) {
					size_t pos = buffer.find_first_of('=');
					if (pos != std::string::npos) {
						propName = buffer.substr(0, pos);
						adjust(propName);
						propValue = buffer.substr(pos + 1, buffer.size());
						adjust(propValue);

						quotemode = false; subString.clear(); tkCount = 0;
						for (const char chara : propValue)
							if (chara == '\"')
								quotemode = !quotemode;
							else if (quotemode)
								subString.append(1, chara);
							else if (chara == split) {
								adjust(subString);
								stackFile.top().get()[propName].setString(subString, tkCount);
								subString.clear();
								tkCount++;
							} else if (chara == '{')
								stackFile.push(stackFile.top().get()[propName]);
							else
								subString.append(1, chara);

						if (!subString.empty()) {
							adjust(subString);
							stackFile.top().get()[propName].setString(subString, tkCount);
						}
					} else if (buffer.find_first_of('{') != std::string::npos){
						propName = buffer.substr(0, buffer.size() - 1);
						adjust(propName);
						stackFile.push(stackFile.top().get()[propName]);
					} else if (buffer[0] == '}')
							stackFile.pop();
					else
							propName = buffer;
					
				}
			}
			file.close();
			return true;
		}
		
		public: inline void setString(const std::string& info, const size_t nItem = 0) {
			if (this->vec_info.size() <= nItem)
				this->vec_info.resize(nItem + 1);
			this->vec_info[nItem] = info;
		}
		public: inline const std::string getString(const size_t nItem = 0) const {
			if (nItem < this->vec_info.size())
				return this->vec_info[nItem];
			return "";
		}
		public: inline void setReal(const double real, const size_t nItem = 0) {
			this->setString(std::to_string(real), nItem);
		}
		public: inline const double getReal(const size_t nItem = 0) {
			return std::atof(this->getString(nItem).c_str());
		}
		public: inline void setInt(const int num, const size_t nItem = 0) {
			this->setString(std::to_string(num), nItem);
		}
		public: inline const int getInt(const size_t nItem = 0) {
			return std::atoi(this->getString(nItem).c_str());
		}
		public: inline size_t getValueCount() const {
			return vec_info.size();
		}
	};
}