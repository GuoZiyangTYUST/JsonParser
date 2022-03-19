#pragma once
// a json read/write library with only one head file  and C++17
// Aim to :
//	(1)be embedded  in the simplest way
//  (2)easy to read and modified
//  Author: Guo ziyang


#include <map>
#include <vector>
#include <variant>
#include <string>
#include <optional>
#include <memory>
#include <fstream>
#include <stack>
#include <cassert>
#include <iostream>
#include <cmath> //for pow

class CJsonDictNode;
class CJsonListNode;

class CJsonNode
{
public:
	virtual ~CJsonNode() {};
	
	virtual void ToString(std::string& OutString,int InLevel=0) = 0;

	virtual bool IsBool()
	{
		return false;
	}
	virtual bool AsBool()
	{
		assert(false&&"this is not a bool node");
		return false;
	}

	virtual bool IsInt()
	{
		return false;
	}

	virtual int AsInt()
	{
		assert(false && "this is not a int node");
		return 0;
	}

	virtual bool IsDouble()
	{
		return false;
	}

	virtual double AsDouble()
	{
		assert(false && "this is not a double node");
		return 0;
	}

	virtual bool IsString()
	{
		return false;
	}

	virtual std::string AsString()
	{
		assert(false && "this is not a string node");
		return std::string();
	}

	virtual bool IsDict()
	{
		return false;
	}

	CJsonDictNode* AsDictNode()
	{
		assert(IsDict() && "this is not a dict node");
		return reinterpret_cast<CJsonDictNode*>(this);
	}

	virtual bool IsList()
	{
		return false;
	}

	CJsonListNode* AsListNode()
	{
		assert(IsList() && "this is not a list node");
		return reinterpret_cast<CJsonListNode*>(this);
	}
};


class CJsonSingleNode :public CJsonNode
{
public:
	using Type = std::variant<int, double, std::string, bool>;
	CJsonSingleNode(const Type& InValue) :Value(InValue) {};

	Type Value;

	virtual void ToString(std::string& OutString, int InLevel = 0) override
	{
		std::visit([&OutString](auto&& InValue)->void {
			using T = std::decay_t<decltype(InValue)>;
			if constexpr (std::is_same_v<T, int>)
				OutString += std::to_string(InValue);
			else if constexpr (std::is_same_v<T, bool>)
				OutString += InValue ? "true" : "false";
			else if constexpr (std::is_same_v<T, double>)
				OutString += std::to_string(InValue);
			else if constexpr (std::is_same_v<T, std::string>)
				OutString += '"' + InValue+'"';
			else
				assert(false && "unimplement !");
			
		}, Value);
	}

	virtual bool IsBool() override
	{
		return std::holds_alternative<bool>(Value);
	}

	virtual bool IsInt() override
	{
		return std::holds_alternative<int>(Value);
	}

	virtual bool IsDouble() override
	{
		return std::holds_alternative<double>(Value);
	}

	virtual bool IsString() override
	{
		return std::holds_alternative<std::string>(Value);
	}
	virtual bool AsBool() override
	{
		assert(IsBool() && "this is not a bool node");
		return std::get<bool>(Value);
	}

	virtual int AsInt() override
	{
		assert(IsInt() && "this is not a int node");
		return std::get<int>(Value);
	}

	virtual double AsDouble() override
	{
		assert(IsDouble() && "this is not a double node");
		return std::get<double>(Value);
	}

	virtual std::string AsString() override
	{
		assert(IsString() && "this is not a string node");
		return std::get<std::string>(Value);
	}
};


class CJsonDictNode :public CJsonNode
{
public:
	std::vector<std::pair<std::string /*NodeName*/ , std::unique_ptr<CJsonNode>>> Value;

	virtual void ToString(std::string& OutString, int InLevel = 0) override
	{
		//OutString += '\n';
		for (int LevelIndex = 0; LevelIndex < InLevel; LevelIndex++)
		{
			OutString += '\t';
		}
		OutString += "{\n";

		size_t ChildeIndex = 0;
		for (auto& Pair : Value)
		{
			for (int LevelIndex = 0; LevelIndex < InLevel+1; LevelIndex++)
			{
				OutString += '\t';
			}

			OutString += '\"';
			OutString += Pair.first;
			OutString += '\"';
			OutString += " : ";
			Pair.second->ToString(OutString, InLevel + 1);

			ChildeIndex++;

			if (ChildeIndex < Value.size())
			{
				OutString += ",";
			}
			OutString += "\n";
		}
		for (int LevelIndex = 0; LevelIndex < InLevel; LevelIndex++)
		{
			OutString += '\t';
		}
		OutString += "}";
	}

	virtual bool IsDict() override
	{
		return true;
	}

};

class CJsonListNode :public CJsonNode
{
public:
	std::vector<std::unique_ptr<CJsonNode>> Value;

	virtual void ToString(std::string& OutString, int InLevel = 0) override
	{
		//OutString += '\n';
		for (int LevelIndex = 0; LevelIndex < InLevel; LevelIndex++)
		{
			OutString += '\t';
		}
		OutString += "[\n";

		for (size_t ChildeIndex = 0; ChildeIndex < Value.size(); ChildeIndex++)
		{
			Value[ChildeIndex]->ToString(OutString, InLevel + 1);

			if (ChildeIndex+1 < Value.size())
			{
				OutString += ",";
			}
			OutString += "\n";
		}

		for (int LevelIndex = 0; LevelIndex < InLevel; LevelIndex++)
		{
			OutString += '\t';
		}
		OutString += "]";
	}

	virtual bool IsList()
	{
		return true;
	}
};


class CJsonUtils
{
public:
	static  std::unique_ptr<CJsonNode> ParseFromFile(std::string JsonFilePath)
	{
		std::ifstream File(JsonFilePath);
		if (!File.is_open())
		{
			assert(false);
			return nullptr;
		}
		//std::string FileContent;
		//File >> FileContent;
		std::string FileContent((std::istreambuf_iterator<char>(File)),
			(std::istreambuf_iterator<char>()));
		return Parse(FileContent);
	}

	static  std::unique_ptr<CJsonNode> Parse(std::string JsonFileContent)
	{
		try
		{
			return CParser(JsonFileContent).Root;
		}
		catch(std::string Error)
		{
			std::cout << "[Json]£ºParse failed for " <<Error<< std::endl;
			return nullptr;
		}
	}


	static  void SaveTo(CJsonNode* InJson, std::string JsonFilePath)
	{
		assert(InJson);
		std::string FileContent;
		InJson->ToString(FileContent);

		std::ofstream File(JsonFilePath, std::ios::out);
		File << FileContent;
	}

private:

	class CParser
	{
	public:
		CParser(const std::string& InJson):Json(InJson)
		{
			if (Json.length() > 0)
			{
				Root=ParseANode();
			}
			
			if(!Root)
			{
				RaiseError("Empty json!");
			}
		}

		std::unique_ptr<CJsonNode> Root = nullptr;
		const std::string& Json;
		size_t CharIndex = 0;
		int Row = 1;
		int Column = 1;

		std::stack<std::unique_ptr<CJsonNode>> Nodes;

		void RaiseError(std::string Error)
		{
			throw Error + " at row " + std::to_string(Row) + " column " + std::to_string(Column);
		}

		int SkipWhiteSpace()
		{
			int Count = 0;
			while (IsA(' ') || IsA('\n'))
			{
				Count++;
				ConsumeCharacter();
			}
			return Count;
		}

		bool IsA(char InChar)
		{
			if (Json[CharIndex] == '\0')
			{
				RaiseError("json is not closed well£¬maybe lost },] or \" !");
			}
			return Json[CharIndex] == InChar;
		}

		bool IsNumber()
		{
			return Json[CharIndex] >='0' && Json[CharIndex] <= '9';
		}

		void ConsumeCharacter()
		{
			if (Json[CharIndex] != '\n')
			{
				Column++;
			}
			else
			{
				Row++;
				Column = 1;
			}
			CharIndex++;
		}

		//return consumed characters
		std::string ConsumeCharacterUntil(char InChar)
		{
			std::string Consumed;
			while (!IsA(InChar))
			{
				Consumed += Json[CharIndex];
				ConsumeCharacter();
			}
			return Consumed;
		}

		std::unique_ptr<CJsonNode> ParseANode()
		{
			SkipWhiteSpace();

			if (IsA('{'))
			{

				Nodes.push(std::make_unique<CJsonDictNode>());
				ConsumeCharacter();
				
				//size_t BeforeParseChild = Nodes.size();
				
				//parse children
				while(!IsA('}'))
				{
					if (SkipWhiteSpace() > 0)
					{
						continue;
					}

					//parse child 
					if (!IsA('"'))
					{
						// parse child name failed
						RaiseError("expected a '\"' !");
					}
					ConsumeCharacter(); //consume first "

					std::string NodeName = ConsumeCharacterUntil('"');
					ConsumeCharacter(); //consume second "

					SkipWhiteSpace(); 

					if (!IsA(':'))
					{
						// parse child name failed
						RaiseError("expected a ':' !");
					}
					ConsumeCharacter();

					static_cast<CJsonDictNode*>(Nodes.top().get())
						->Value.push_back(std::make_pair(NodeName, ParseANode()));

					SkipWhiteSpace();
					if (IsA(','))
					{
						ConsumeCharacter();
					}

				}
				ConsumeCharacter(); //consume }

				//size_t AfterParseChild = Nodes.size();
				//assert(BeforeParseChild == AfterParseChild);

				std::unique_ptr<CJsonNode> Node(Nodes.top().release());
				Nodes.pop();
				return Node;

			}
			else if (IsA('"'))
			{
				ConsumeCharacter(); //consume first " of name
				
				std::string Value = ConsumeCharacterUntil('"');

				if (!IsA('"'))
				{
					// parse child name failed
					RaiseError("expected a '\"' !");
				}
				ConsumeCharacter(); //consume second " of name

				return std::make_unique<CJsonSingleNode>(Value);
			}
			else if (IsA('.') || IsNumber() || IsA('-'))
			{
				std::string NumerInString;
				bool IsInt = true;
				while (IsA('.') || IsNumber() || IsA('-'))
				{
					if (IsA('.'))
					{
						IsInt = false;
					}
					NumerInString += Json[CharIndex];
					ConsumeCharacter();
				}
				
				double Multiple = 1;
				if (IsA('e') || IsA('E'))
				{
					ConsumeCharacter();

					std::string Exponent;
					while (IsNumber() || IsA('-'))
					{
						Exponent += Json[CharIndex];
						ConsumeCharacter();
					}	 
					Multiple=std::powf(10, (float)std::stoi(Exponent));
				}

				if (IsInt)
				{
					return std::make_unique<CJsonSingleNode>(int(std::stoi(NumerInString) * Multiple));
				}
				else
				{
					return std::make_unique<CJsonSingleNode>(std::stod(NumerInString)* Multiple);
				}
			}
			else if (IsA('['))
			{
				ConsumeCharacter();
				Nodes.push(std::make_unique<CJsonListNode>());
				
				//size_t BeforeParseChild = Nodes.size();

				while (!IsA(']'))
				{
					if (SkipWhiteSpace() > 0)
					{
						continue;
					}

					static_cast<CJsonListNode*>(Nodes.top().get())
						->Value.push_back(ParseANode());

					SkipWhiteSpace();

					if (IsA(','))
					{
						ConsumeCharacter();
					}
					//donot request there must be a JsonNode after a ','. this  parse is not so serious
				}
				ConsumeCharacter(); //consume ']'

				//size_t AfterParseChild = Nodes.size();
				//assert(BeforeParseChild == AfterParseChild);

				std::unique_ptr<CJsonNode> Node(Nodes.top().release());
				Nodes.pop();
				return Node;
			}
			else if (IsA('f') )
			{
				ConsumeCharacter();
				if (!IsA('a'))
				{
					RaiseError("may written 'false' incorrectly  !");
				}
				ConsumeCharacter();
				if (!IsA('l'))
				{
					RaiseError("may written 'false' incorrectly  !");
				}
				ConsumeCharacter();
				if (!IsA('s'))
				{
					RaiseError("may written 'false' incorrectly  !");
				}
				ConsumeCharacter();
				if (!IsA('e'))
				{
					RaiseError("may written 'false' incorrectly  !");
				}
				ConsumeCharacter();


				return std::make_unique<CJsonSingleNode>(false);
			}
			else if (IsA('t'))
			{
				ConsumeCharacter();
				if (!IsA('r'))
				{
					RaiseError("may written 'false' incorrectly  !");
				}
				ConsumeCharacter();
				if (!IsA('u'))
				{
					RaiseError("may written 'false' incorrectly  !");
				}
				ConsumeCharacter();
				if (!IsA('e'))
				{
					RaiseError("may written 'false' incorrectly  !");
				}
				ConsumeCharacter();

				return std::make_unique<CJsonSingleNode>(true);
			}

			return nullptr;
		}
	};

};



