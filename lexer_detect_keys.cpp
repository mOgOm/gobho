#include "lexer_detect_keys.h"

using shp_t = std::shared_ptr<Token>;

namespace lexer_detect_keys_private
{
	bool removeUnPlusses (std::vector<shp_t>& tokens, Tok type, size_t& idx);
	bool detectUnMinuses (std::vector<shp_t>& tokens, Tok type, size_t& idx);
	bool detectTypesOfNumConst (std::vector<shp_t>& tokens, Tok type, size_t& idx);
	
	std::vector<Tok>* p_parenthesisStack{nullptr};
	std::pair<Tok, size_t>* p_errLog{nullptr};
	bool countParenthesis (Tok type, size_t parReg[6], size_t& idx);
	bool err = false;

	// После инициализации, стек глубины последнего ключевого слова (по круглым скобкам)
	std::vector<size_t>* key_dp_st_pb{nullptr};

	// После инициализации, массив счётчиков скобок (:0 ):0 [:0 ]:0 {:0 }:0
	size_t* p_reg{nullptr};

	// После вызова loadKeyWords, мапа из ключевого слова, в токен
	std::map<std::string, Tok> keyWords;

	enum class WasReplased {
		Nothing,
		Lambda,
		NotLambda,
	};
}


/*
## Loads key words from path ##

Needs to be called before lexering and parsing source code.
Expected that file in path contain list of pairs:

[key_word, local_key_word]
[key_word, local_key_word]
[key_word, local_key_word]...
*/
bool loadKeyWords (const char path[])
{
	lexer_detect_keys_private::keyWords.clear ();
	using namespace lexer_detect_keys_private;
	bool res = true;
	enum Ord {FIRST, SECOND} order{Ord::FIRST};
	std::vector <std::shared_ptr<Token> > keyWords_toks;
	std::shared_ptr<Token> p{nullptr};
	lexer_1 (path, keyWords_toks);

	std::map<std::string ,Tok> vals {
		{"lambda", Tok::Lambda},
		{"if", Tok::If},
		{"else", Tok::Else},
		{"elif", Tok::Elif},
		{"try", Tok::Try},
		{"catch", Tok::Catch},
		{"for", Tok::For},
		{"while", Tok::While},
		{"return", Tok::Return},
		{"assert", Tok::Assert},
		{"let", Tok::Let},
		{"throw", Tok::Throw},
		{"and", Tok::And},
		{"or", Tok::Or},
		{"not", Tok::Not},
		{"break", Tok::Break},
		{"const", Tok::Const},
		{"import", Tok::Import},
		{"as", Tok::As},
		{"delete", Tok::Delete},
		{"continue", Tok::Continue},
		{"i16", Tok::I16_type},
		{"i32", Tok::I32_type},
		{"i64", Tok::I64_type},
		{"u16", Tok::U16_type},
		{"u32", Tok::U32_type},
		{"u64", Tok::U64_type},
		{"f32", Tok::F32_type},
		{"f64", Tok::F64_type},
		{"string", Tok::String_type},
		{"array", Tok::Array_type},
		{"map", Tok::Map_type},
		{"vector", Tok::Vector_type},
		{"atomic", Tok::Atomic_type},
		{"shared_ptr", Tok::SharedPtr_type},
		{"template", Tok::TemplateKey},
		{"typename", Tok::Typename}
	};

	size_t idx = 0;
	std::string buff = "";

	for (auto it : keyWords_toks) {

		if (it->type () == Tok::Id) {
			if (order == Ord::FIRST) {
				buff = as<Id_t> (it)->getStr ();
				p = it;
				order = Ord::SECOND;
			} else {
				keyWords[as<Id_t> (it)->getStr ()] = vals[buff];
				order = Ord::FIRST;
				++idx;
			}
		}
	}

	return res;
}

/*
## Replaces id token with matching key-word token ##
	
Takes reference to container of tokens, index of token to be
replaced to be changed and type of token that needs to be set.
*/
lexer_detect_keys_private::WasReplased
	replace (std::vector<shp_t>& tokens, size_t& idx, Tok t)
{
	using namespace lexer_detect_keys_private;

	WasReplased res = WasReplased::Nothing;
	auto notLambdaRepl = [&res]() {res = WasReplased::NotLambda;};

	switch (t) {

	case Tok::Lambda:
		tokens[idx] = shp_t (new Lambda_t ());
		res = WasReplased::Lambda;
		break;

		
	case Tok::Else:
		tokens[idx] = shp_t (new ElseKey_t ());
		tokens.insert (std::begin (tokens) + idx, shp_t (new CloseBody_t ()));
		++idx;
		notLambdaRepl ();
		break;
		
	case Tok::Elif:
		tokens[idx] = shp_t (new ElifKey_t ());
		tokens.insert (std::begin (tokens) + idx, shp_t (new CloseBody_t ()));
		++idx;
		notLambdaRepl ();
		break;

	case Tok::If:              tokens[idx] = shp_t (new IfKey_t          ());    notLambdaRepl (); break;
	case Tok::Try:             tokens[idx] = shp_t (new Try_t            ());    notLambdaRepl (); break;
	case Tok::Catch:           tokens[idx] = shp_t (new Catch_t          ());    notLambdaRepl (); break;
	case Tok::While:           tokens[idx] = shp_t (new WhileKey_t       ());    notLambdaRepl (); break;
	case Tok::Return:          tokens[idx] = shp_t (new Return_t         ());    notLambdaRepl (); break;
	case Tok::Assert:          tokens[idx] = shp_t (new Assert_t         ());    notLambdaRepl (); break;
	case Tok::Let:             tokens[idx] = shp_t (new Let_t            ());    notLambdaRepl (); break;
	case Tok::Throw:           tokens[idx] = shp_t (new Throw_t          ());    notLambdaRepl (); break;
	case Tok::And:             tokens[idx] = shp_t (new And_t            ());    notLambdaRepl (); break;
	case Tok::Or:              tokens[idx] = shp_t (new Or_t             ());    notLambdaRepl (); break;
	case Tok::Not:             tokens[idx] = shp_t (new Not_t            ());    notLambdaRepl (); break;
	case Tok::Break:           tokens[idx] = shp_t (new Break_t          ());    notLambdaRepl (); break;
	case Tok::Const:           tokens[idx] = shp_t (new Const_t          ());    notLambdaRepl (); break;
	case Tok::Import:          tokens[idx] = shp_t (new Import_t         ());    notLambdaRepl (); break;
	case Tok::As:              tokens[idx] = shp_t (new As_t             ());    notLambdaRepl (); break;
	case Tok::Empty:           tokens[idx] = shp_t (new Empty_t          ());    notLambdaRepl (); break;
	case Tok::Delete:          tokens[idx] = shp_t (new Delete_t         ());    notLambdaRepl (); break;
	case Tok::Continue:        tokens[idx] = shp_t (new Continue_t       ());    notLambdaRepl (); break;
	case Tok::I16_type:        tokens[idx] = shp_t (new I16_type_t       ());    notLambdaRepl (); break;
	case Tok::I32_type:        tokens[idx] = shp_t (new I32_type_t       ());    notLambdaRepl (); break;
	case Tok::U32_type:        tokens[idx] = shp_t (new U32_type_t       ());    notLambdaRepl (); break;
	case Tok::F64_type:        tokens[idx] = shp_t (new F64_type_t       ());    notLambdaRepl (); break;
	case Tok::Typename:        tokens[idx] = shp_t (new Typename_t       ());    notLambdaRepl (); break;
	case Tok::SharedPtr_type:  tokens[idx] = shp_t (new SharedPtr_type_t ());    notLambdaRepl (); break;
	case Tok::UniquePtr_type:  tokens[idx] = shp_t (new UniquePtr_type_t ());    notLambdaRepl (); break;
	case Tok::Atomic_type:     tokens[idx] = shp_t (new Atomic_type_t    ());    notLambdaRepl (); break;
	case Tok::String_type:     tokens[idx] = shp_t (new String_type_t    ());    notLambdaRepl (); break;
	
	case Tok::For: {

		tokens[idx] = shp_t (new ForKey_t ());

		for (size_t jdx = idx + 1; jdx < tokens.size (); ++jdx) {

			Tok type = tokens[jdx]->type ();

			if (type == Tok::Colon || type == Tok::In) {
				tokens[jdx].reset (new In_t ());
				break;
			}

			if (type != Tok::Id) // erase not id-s
				tokens.erase (std::begin (tokens) + (jdx--));
		}
		notLambdaRepl ();

		break;
	}

	default: break;
	}

	return res;
}

size_t findCloser (std::vector<shp_t>& tokens, size_t jdx) {
	const auto size = tokens.size ();
	unsigned depth = 1;
	size_t kdx = jdx + 1;
	for (; depth != 0 && kdx < size; ++kdx) {
		const auto type = tokens[kdx]->type ();
		if (isOpener (type) || type == Tok::OpenLambdaType || type == Tok::OpenTemplateType) ++depth;
		else if (isCloser (type) || type == Tok::CloseLambdaType || type == Tok::CloseTemplateType) --depth;
	}
	return kdx;
}

/*	## Second phase of lexering code ##

	- replase id tokens with matching key-word tokens
	- replase num tokens (values in wstring) with f32/i32/u32 tokens
	- replase id!() and \() with template and lambda parenthesis
	- replase nye token with not_token
	- set type-flag tokens (where starts/ends type)
	- check for brackets order
	- replase dots and commas (after key-word tokens)
	  with open-body token
*/
bool lexer_2 (std::vector<std::shared_ptr<Token> >& tokens)
{
	using namespace lexer_detect_keys_private;

	using shp_t = std::shared_ptr<Token>;
	
	enum typeOfStart{
		notReady,
		readyForComma,
		readyForDot
	}
	readyToStartBody = notReady;

	std::vector<int> workOnTypeStack;
	workOnTypeStack.reserve (2);

	size_t reg[6] = {0, 0, 0, 0, 0, 0}; // (:0 ):0 [:0 ]:0 {:0 }:0
	p_reg = reg;

	std::vector<size_t> keyDepthStack;
	keyDepthStack.reserve (8);
	std::vector<Tok> parenthsisStack;
	p_parenthesisStack = &parenthsisStack;
	std::pair<Tok, std::size_t> errLog {Tok::OpenBody, std::wstring::npos};
	p_errLog = &errLog;
	key_dp_st_pb = &keyDepthStack;
	size_t template_depth = 0;


	for (size_t idx = 0, depth = 0; idx < tokens.size (); ++idx) {
		if (errLog.second != std::wstring::npos) break;
		
		auto type = tokens[idx]->type();
		
		// Пропускаем все пробелы
		if (type == Tok::Space) continue;

		// Учли скобки
		if (countParenthesis (type, reg, idx)) {
			if (!workOnTypeStack.empty()) {
				if (type == Tok::CloseParenthesis && workOnTypeStack.back() - 1 == reg[0] - reg[1]) {
					tokens.insert (std::begin (tokens) + idx, shp_t (new EndType_t ()));
					workOnTypeStack.pop_back ();
					++idx;
				}
			}
			if (errLog.second != std::wstring::npos) break; // Ошибка, если нарушен порядок скобок
			continue;
		}
		// Определили тип числа
		if (detectTypesOfNumConst (tokens, type, idx)) continue;

		// если встретилась точка и мы готовы к началу тела
		if (type == Tok::Dot){ 
			//if (workOnTypeStack.empty ()) // если не работаем над типом
			//{
			if(readyToStartBody == readyForDot && depth == reg[0] - reg[1] && template_depth == 0) {
				tokens[idx] = shp_t(new OpenBodyOfLambda_t()); // заменили точку на открытие тела лямбда выражения
				if (workOnTypeStack.size () == 1) {
					workOnTypeStack.clear ();
					tokens.insert (std::begin (tokens) + idx, shp_t (new EndType_t ()));
					++idx;
					readyToStartBody = notReady;
				}
				continue;
			}
			//} else { // точка не может встретиться в типе.
			// Для доступа к статическим данным (и к типам)
			// используется оператор разрешения области видимости ::
			//}
		}

		// если нам встретилась запятая и текущая глубина,
		// та же, что и у последнего ключевого слова
		if (type == Tok::Comma) {
			if (!workOnTypeStack.empty ()){
				if (workOnTypeStack.size() == 1) {
					tokens[idx] = shp_t(new EndType_t());
					workOnTypeStack.pop_back();
				} else {
					tokens[idx] = shp_t (new TypeComma_t());
				}
			}

			if(!keyDepthStack.empty () && keyDepthStack.back () == reg[0] - reg[1]) {
				keyDepthStack.pop_back();
				tokens[idx] = shp_t(new OpenBody_t()); // заменили запятую на открытие тела
				readyToStartBody = notReady;
			}
			continue;
		}

		if (type == Tok::Colon && workOnTypeStack.empty ()){
			if (readyToStartBody == readyForDot) {
				// если встретили двоеточие при объявлении лямбды, то это начало типа
				tokens[idx] = shp_t (new StartType_t ());
				workOnTypeStack.push_back (reg[0] - reg[1]);
			} else { // иначе дожидаемся присвоения (тогда всё это тип) или точки с запятой (это не было типом)
				auto jdx = idx;
				Tok type = Tok::Colon;
				auto size = tokens.size () - 1;
				unsigned depth = 0;
				while (jdx < size) {
					++jdx;
					type = tokens[jdx]->type ();
					if (isOpener (type)) ++depth;
					else if (isCloser (type)) {if (depth == 0) break; --depth;}
					if (depth == 0 && (type == Tok::Assign || type == Tok::Semicolon)) break;
				}
				if (type == Tok::Assign) {
					tokens[idx] = shp_t (new StartType_t ());
					workOnTypeStack.push_back (reg[0] - reg[1]);
				}
			}
			continue;
		}

		if (readyToStartBody == readyForDot){
			if (type == Tok::Sword || type == Tok::Arrow) {
				// если меч, то далее будет возвращаемый тип
				if (!workOnTypeStack.empty ()) {
					if (workOnTypeStack.back () == reg[0] - reg[1]) {
						workOnTypeStack.pop_back ();
					}
				}
				workOnTypeStack.push_back (reg[0] - reg[1]);
				tokens.insert (std::begin (tokens) + idx - 1, shp_t (new EndType_t ()));
				idx += 2;
				tokens.insert (std::begin(tokens) + idx, shp_t (new StartType_t()));
				continue;
			}

			/*if ( type == Tok::Plus // \ х :ц32 -+ ц32 +. х * 2; --- лямбда принимающая и возвращающая int32
				//&& readyToStartBody == readyForDot
				&& (keyDepthStack.empty () || keyDepthStack.back () == reg[0] - reg[1])
				&& !workOnTypeStack.empty ())
			{
				workOnTypeStack.pop_back ();
				tokens[idx] = shp_t (new EndType_t ()); // описание типа завершается вместо плюса
			}//*/
		}

		if ( (type == Tok::Assign)
			&& (workOnTypeStack.empty () || workOnTypeStack.back () == reg[0] - reg[1]))
			// 	х :ц32 = 10; --- связывание х с целым 32-битным числом 
		{	//	foo :(ц32)ц32 = \ х :ц32 -+ ц32 +. х; --- связывание foo с
			//	--- лямбда выражением принимающим целое 32-битное число
			//	--- и возврающим целое 32-битное число
			// описание типа заканчивается перед знаком связывания
			if (!workOnTypeStack.empty ()) {
				tokens.insert (std::begin (tokens) + idx, shp_t (new EndType_t));
				workOnTypeStack.pop_back ();
			}
			++idx;
			continue;
		}

		// заменил '!('  и  ')' на символы открытия и закрытие шаблона
		if (type == Tok::nye) {
			bool allright = false;
			if (idx != 0) {
				size_t ldx = idx-1;
				while (ldx != 0 && tokens[ldx]->type () == Tok::Space) --ldx;
				allright = tokens[ldx]->type () == Tok::Id;
			}
			if (allright) {
				size_t jdx = idx + 1;
				const auto size = tokens.size ();
				while (jdx < size && tokens[jdx]->type () == Tok::Space) ++jdx;
				if (jdx == size) throw "Error! There is must to be '(' token after type template token";
				if (tokens[jdx]->type () == Tok::OpenParenthesis) {
					size_t kdx = findCloser (tokens, jdx) - 1;
					if (kdx == tokens.size ()) throw "Error! There is must to be ')' token to close template arguments";
					tokens[jdx] = shp_t (new OpenTemplateType_t ());
					tokens[kdx] = shp_t (new CloseTemplateType_t ());
					tokens.erase (std::begin (tokens) + idx);
					workOnTypeStack.push_back (reg[0] - reg[1]);
					++template_depth;
					++idx;
				}
			} else {
				tokens[idx] = shp_t (new Not_t ());
			}
			continue;
		} 

		if (type == Tok::CloseTemplateType) {
			assert (template_depth > 0);
			workOnTypeStack.pop_back ();
			--template_depth;
		}

		if (type == Tok::CloseLambdaType) {
			workOnTypeStack.pop_back ();
		}

		// Определили унарные минусы
		if (detectUnMinuses (tokens, type, idx)) continue;

		// Убрали унарные плюсы
		if (removeUnPlusses (tokens, type, idx)) continue;

		if (type == Tok::Lambda) {
			if (workOnTypeStack.empty ()) {// если наткнулись на лямбду не в типе, то ожидаем точку на текущей глубине
				readyToStartBody = readyForDot;
				depth = reg[0] - reg[1];
			}
			else { // если работаем над типом, то стоит поменять  знак лямбды и последующую скобку на открытие лямбда-типа
				size_t jdx = idx + 1;
				const auto size = tokens.size ();
				while (jdx < size && tokens[jdx]->type () == Tok::Space) ++jdx;

				if (jdx == size || tokens[jdx] -> type() != Tok::OpenParenthesis)
					throw "Error! There is must to be '(' token after type lambda token";
				
				size_t local_depth = 1;
				auto kdx = findCloser (tokens, jdx) - 1;

				// [idx]->\ [jdx]->( ... )<-[kdx]
				tokens[jdx] = shp_t (new OpenLambdaType_t ());
				tokens[kdx] = shp_t (new CloseLambdaType_t ());
				tokens.erase (std::begin (tokens) + idx);
				workOnTypeStack.push_back (reg[0] - reg[1]);
				++idx;
			}
			continue;
		}
		if (type == Tok::Id) {
			const auto&& id = as<Id_t>(tokens[idx]) -> getStr ();

			if (keyWords.count (id)) {
				
				WasReplased was_replased = replace (tokens, idx, keyWords[id]); // заменили тип ноды с id на ключевое слово
				
				// Если лямбда
				if (was_replased == WasReplased::Lambda) {
					// готовимся обработать не вложенную в скобки точку
					readyToStartBody = readyForDot; depth = reg[0] - reg[1];
				}
				if (was_replased == WasReplased::NotLambda) { // запомнили 'глубину по круглым скобкам' для ключевого слова
					if (key_dp_st_pb != nullptr && p_reg != nullptr) {
						key_dp_st_pb->push_back (p_reg[0] - p_reg[1]);
					}
				}
				continue;
			}
		}
	}

	bool res = true;

	// Вывод текста ошибок
/*
	if (errLog.second != std::wstring::npos) {
		const auto par_type = errLog.first;
		std::string par_type_s;
		switch (par_type) {
			case Tok::CloseParenthesis: par_type_s = "znpyzfzqaZN"; break;
			case Tok::CloseBrackets: par_type_s = "znzdazgpaztzraZN"; break;
			case Tok::CloseCurlyBrackets: par_type_s = "zVzlzfypzraZN"; break;
			default: break;
		}
		std::wcerr << "===---- " << latToCir ("OZdzlzDzna. zQzlZdzrZNZN zkaznpZIzdaZlZfaZN " + par_type_s + " cznozDzna zra cztpozne ")
			<< std::to_wstring (tokens[errLog.second]->getLine () + 1)
			<< " ----===" << std::endl;
		res = false;
	}
	else if (!parenthsisStack.empty ()) {
		unsigned idx = 0, c = 0;
		const auto par_type = parenthsisStack.back ();
		for (auto& it : parenthsisStack) if (par_type == it) ++c;
		for (; idx < tokens.size () && c != 0; ++idx) if (tokens[idx]->type() == par_type) --c;
		std::string par_type_s;
		switch (par_type) {
			case Tok::OpenParenthesis: par_type_s = "znpyzfzqaZN"; break;
			case Tok::OpenBrackets: par_type_s = "znzdazgpaztzraZN"; break;
			case Tok::OpenCurlyBrackets: par_type_s = "zVzlzfypzraZN"; break;
			default: break;
		}
		std::wcerr << "===---- " << latToCir ("OZdzlzDzna. ")
			<< latToCir ("HezkaznpZIztaZN " + par_type_s + " cznozDzna zra cztpozne ")
			<< std::to_wstring (tokens[idx]->getLine () + 1)
			<< " ----===" << std::endl;
		res = false;
	}
//*/

	key_dp_st_pb = nullptr;
	p_reg = nullptr;

	return res;
}


namespace lexer_detect_keys_private
{
	bool detectTypesOfNumConst (std::vector<std::shared_ptr<Token> >& tokens, Tok type, size_t& idx)
	{
		bool done = false;
		if (type == Tok::Number) {
			std::wstring&& id = tokens[idx]->getId ();
			if (id.find (L'.') == std::wstring::npos) {
				tokens[idx] = std::shared_ptr<Token> (new Int_t ());
				tokens[idx]->setInt (std::stoi (id));
			} else {
				float f;
				std::wstringstream s;
				s << id, s >> f;
				tokens[idx] = std::shared_ptr<Token> (new Float_t ());
				tokens[idx]->setFloat (f);
			}
			done = true;
		}
		return done;
	}

	bool detectUnMinuses (std::vector<std::shared_ptr<Token> >& tokens, Tok type, size_t& idx)
	{
		bool done = false;
		if (type == Tok::Minus) {
			if (idx > 0) {
				for (auto jdx = idx - 1;; jdx -= 1) {

					auto jtype = tokens[jdx]->type ();

					if (jtype == Tok::Space) {
						if (jdx == 0) break;
						continue;
					}

					bool needToStop =
						jtype == Tok::Int || jtype == Tok::Float || jtype == Tok::Id ||
						jtype == Tok::CloseBrackets || jtype == Tok::CloseParenthesis ||
						jtype == Tok::CloseCurlyBrackets;

					if (!needToStop)
						tokens[idx] = std::shared_ptr<Token> (new UnMinus_t ());
					
					break;
				}
			}
			else
				tokens[idx] = std::shared_ptr<Token> (new UnMinus_t ());
			
			done = true;
		}
		return done;
	}

	bool removeUnPlusses (std::vector<std::shared_ptr<Token> >& tokens, Tok type, size_t& idx)
	{
		bool done = false;
		if (type == Tok::Plus) {
			if (idx > 0) {
				for (auto jdx = idx - 1;; jdx -= 1) {
					
					auto jtype = tokens[jdx]->type ();

					if (jtype == Tok::Space) {
						if (jdx == 0) break;
						continue;
					}
					bool needToStop = jtype == Tok::Int || jtype == Tok::Float || jtype == Tok::Id || jtype == Tok::CloseBrackets || jtype == Tok::CloseParenthesis || jtype == Tok::CloseCurlyBrackets;
					if (!needToStop) {
						tokens.erase (std::begin (tokens) + idx);
						--idx;
					}
					break;
				}
			}
			else {
				tokens.erase (std::begin (tokens) + idx);
				--idx;
			}
			done = true;
		}
		return done;
	}

	bool countParenthesis (Tok type, size_t parReg[6], size_t& idx)
	{
		if (type == Tok::OpenParenthesis) { ++parReg[0]; p_parenthesisStack->push_back (type); return true; }
		if (type == Tok::CloseParenthesis) {
			++parReg[1];
			if (p_parenthesisStack->empty() || p_parenthesisStack->back () != Tok::OpenParenthesis) (*p_errLog) = std::pair<Tok, size_t>(type, idx);
			if (!p_parenthesisStack->empty ()) p_parenthesisStack->pop_back ();
			return true;
		}
		if (type == Tok::OpenBrackets) { ++parReg[2]; p_parenthesisStack->push_back (type); return true; }
		if (type == Tok::CloseBrackets) {
			++parReg[3];
			if (p_parenthesisStack->empty () || p_parenthesisStack->back () != Tok::OpenBrackets) (*p_errLog) = std::pair<Tok, size_t> (type, idx);
			return true;
		}
		if (type == Tok::OpenCurlyBrackets) { ++parReg[4]; p_parenthesisStack->push_back (type); return true; }
		if (type == Tok::CloseCurlyBrackets) {
			++parReg[5];
			if (p_parenthesisStack->empty () || p_parenthesisStack->back () != Tok::OpenParenthesis) {
				(*p_errLog) = std::pair<Tok, size_t> (type, idx);
			}
			return true;
		}
		return false;
	}
}
