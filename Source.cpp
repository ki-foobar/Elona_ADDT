#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define ADDT_LINE_NUMBER_MAX 1024

#define ADDT_EXPORT extern "C" __declspec (dllexport)

#define equals(s1,s2) (strcmp(s1,s2) == 0)

#define gdata(i) intarray(pr->data->hei,pr->data->gdataPVal,i)
#define cdata(i,j) intmap(pr->data->hei,pr->data->cdataPVal,i,j)
#define adata(i,j) intmap(pr->data->hei,pr->data->adataPVal,i,j)
#define sdata(i,j) intmap(pr->data->hei,pr->data->sdataPVal,i,j)
#define mdata(i) intarray(pr->data->hei,pr->data->mdataPVal,i)
#define cdatan(i,j) strmap(pr->data->hei,pr->data->cdatanPVal,i,j)
#define rattach(dbl) rankflag ? rankattach(dbl) : (dbl)
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))


// 'strlen(string_literal)' is surely expected to be calculated in compile time.
#define Token_equals_literal(token, string_literal) \
	Token_equals_with_length((token), (string_literal), strlen(string_literal))

#define Token_starts_with_literal(token, string_literal) \
	Token_starts_with_with_length((token), (string_literal), strlen(string_literal))


#define StrComparisonValue_set_integer(strcomparisonvalue, value) \
	do { \
		sprintf(static_buffer, "%d", (value)); \
		parse_result->str = static_buffer; \
		parse_result->len = strlen(static_buffer); \
	} while (0)

#define StrComparisonValue_set_string(strcomparisonvalue, value) \
	do { \
		const char *tmp = (value); \
		(strcomparisonvalue)->str = (tmp); \
		(strcomparisonvalue)->len = strlen(tmp); \
	} while (0)

#define StrComparisonValue_set_string_literal(strcomparisonvalue, value) \
	do { \
		(strcomparisonvalue)->str = (value); \
		(strcomparisonvalue)->len = strlen(value); \
	} while (0)



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>
#include "hsp3plugin.h"


typedef struct {
	HSPEXINFO *hei;
	char *buff;			// 第1パラメータ buff@m116
	PVal *gdataPVal;	// 第2パラメータ gdata
	PVal *cdataPVal;	// 第3パラメータ cdata
	PVal *sdataPVal;	// 第4パラメータ sdata
	PVal *adataPVal;	// 第5パラメータ adata
	PVal *mdataPVal;	// 第6パラメータ mdata
	PVal *cdatanPVal;	// 第7パラメータ cdatan
	PVal *invPVal;		// 第8パラメータ inv
	PVal *addtlvarPVal;	// 第9パラメータ addtlvar
	char *addtgvar;		// 第10パラメータ addtgvar
	int unitid;			// 第11パラメータ unitid
	int tcid;			// 第12パラメータ tc
	int ccid;			// 第13パラメータ cc
	char *filter;		// 第14パラメータ 条件式フィルタ
} args;


typedef struct {
	const char *str;
	args *data;
} Parser;

typedef enum {
	TOKEN_TYPE_ERROR,
	TOKEN_TYPE_LEFT_PAREN,
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_INTEGER,
} TokenType;

typedef struct {
	TokenType type;
	char* str;
	size_t len;
} Token;

typedef enum {
	RANGE_MATCHER_TYPE_EXACT,
	RANGE_MATCHER_TYPE_LESS,
	RANGE_MATCHER_TYPE_GREATER,
	RANGE_MATCHER_TYPE_BETWEEN,
} RangeMatcherType;

typedef struct {
	RangeMatcherType type;
	int value0;
	int value1;
} RangeMatcher;

const bool RANGE_MATCHER_PARSE_MODE_ACCEPT_OPEN_RANGE = true;
const bool RANGE_MATCHER_PARSE_MODE_DENY_OPEN_RANGE = false;

const bool RANGE_MATCHER_PARSE_MODE_ACCEPT_FLOOR_NOTATION = true;
const bool RANGE_MATCHER_PARSE_MODE_DENY_FLOOR_NOTATION = false;

typedef IntComparisonValue double;

typedef enum {
	INTCOMPARISON_OPERATOR_TYPE_EQ,
	INTCOMPARISON_OPERATOR_TYPE_LE,
	INTCOMPARISON_OPERATOR_TYPE_LT,
	INTCOMPARISON_OPERATOR_TYPE_GE,
	INTCOMPARISON_OPERATOR_TYPE_GT,
	INTCOMPARISON_OPERATOR_TYPE_LE_MUL,
	INTCOMPARISON_OPERATOR_TYPE_GE_MUL,
	INTCOMPARISON_OPERATOR_TYPE_LE_ADD,
	INTCOMPARISON_OPERATOR_TYPE_GE_ADD,
} IntComparisonOperatorType;

typedef struct {
	IntComparisonOperatorType type;
	IntComparisonValue extra;
} IntComparisonOperator;

typedef struct {
    char* str;
    size_t len;
} StrComparisonValue;

int split(char *str, const char *delim, char *outlist[], int limit) {
	char    *tk;
	int     cnt = 0;

	tk = strtok(str, delim);
	while (tk != NULL && cnt < limit) {
		outlist[cnt++] = tk;
		tk = strtok(NULL, delim);
	}
	return cnt;
}

bool decimalonly(char *p) {
	while (p[0]) {
		if (p[0] == '.' || (p[0] >= '0' && p[0] <= '9')) p++;
		else break;
	}
	if (p[0]) {
		return false;
	}
	else {
		return true;
	}
}

/* 先頭からtabを取り除き、その数を返す */
int removefs(char *s) {
	int i = 0;
	while (s[i] != '\0' && s[i] == '	') i++;
	strcpy(s, &s[i]);

	return i;
}

/* buffにあるsの文字の位置を返す */
int search(char *buff, char s) {
	char *index = buff;
	while (index[0] != s && index[0] != '\0') {
		index++;
	}
	return index - buff;
}

char* chartrim(char *s, char g) {
	if (s == NULL) return s;
	int i = strlen(s);
	int count = 0;
	while (--i >= 0 && s[i] == g) count++;
	s[i + 1] = '\0';

	i = 0;
	while (s[i] != '\0' && s[i] == g) i++;
	strcpy(s, &s[i]);

	return s;
}

bool Token_equals_with_length(const Token *tok, const char *s, size_t s_len) {
	if (tok->len != s_len)
		return false;

	return strncmp(tok->str, s, s_len) == 0;
}

bool Token_equals(const Token *tok, const char *s) {
	return Token_equals_with_length(tok, s, strlen(s));
}

bool Token_starts_with_with_length(const Token *tok, const char *s, size_t s_len) {
	if (tok->len < s_len)
		return false;

	return strncmp(tok->str, s, s_len) == 0;
}

bool Token_starts_with(const Token *tok, const char *s) {
	return Token_starts_with_with_length(tok, s, strlen(s));
}

bool Parser_parse_one_token(Parser *pr, Token *parse_result) {
	Parser_skip_whitespaces(pr);

	parse_result->str = pr->str;

	if (*pr->str == '(') {
		parse_result->type = TOKEN_TYPE_LEFT_PAREN;
		++pr->str;
		parse_result->len = pr->str - parse_result->str;
		return true;
	}

	if (isalpha(*pr->str) || *pr->str == '_') {
		while (isalnum(*pr->str) || *pr->str == '_')
			++pr->str;
		parse_result->type = TOKEN_TYPE_IDENTIFIER;
		parse_result->len = pr->str - parse_result->str;
		return true;
	}

	if (*pr->str == '+' || *pr->str == '-' || isnum(*pr->str)) {
		if (*pr->str == '+' || *pr->str == '-')
			++pr->str;
		while (isnum(*pr->str))
			++pr->str;
		parse_result->type = TOKEN_TYPE_INTEGER;
		parse_result->len = pr->str - parse_result->str;
		return true;
	}

	parse_result->len = 0;
	return false;
}

bool Parser_parse_one_token_of(Parser *pr, Token *parse_result, TokenType token_type) {
	return Parser_parse_one_token(pr, &parse_result) && parse_result->type == token_type;
}

bool Parser_parse_integer(Parser *pr, int *parse_result, bool accept_floor_notation) {
	int sign = 1;
	if (*pr->str == 'B') {
		if (!accept_floor_notation)
			return false;

		sign = -1;
		++pr->str;
	}

	Token tok;
	if (!Parser_parse_one_token_of(pr, &tok, TOKEN_TYPE_INTEGER))
		return false;

	*parse_result = sign * atoi(tok.str);
	return true;
}

bool Parser_parse_floating(Parser *pr, double *parse_result) {
	Token tok;
	if (!Parser_parse_one_token_of(pr, &tok, TOKEN_TYPE_INTEGER))
		return false;

	*parse_result = atof(tok.str);
	return true;
}

/**
 * 1) n       (closed)
 * 2) - n     (open)
 * 3) n -     (open)
 * 4) n - m   (closed)
 */
bool Parser_parse_range_matcher_internal(
		Parser *pr,
		RangeMatcher *parse_result,
		bool accept_open_range,
		bool accept_floor_notation) {
	Parser_skip_whitespaces(pr);

	// 2) - n
	if (*pr->str == '-' && !isnum(pr->str[1])) {
		if (!accept_open_range)
			return false;

		++pr->str;
		Parser_skip_whitespaces(pr);

		int value0;
		if (!Parser_parse_integer(pr, &value0, accept_floor_notation))
			return false;

		parse_result->type = RANGE_MATCHER_TYPE_LESS;
		parse_result->value0 = value0;
		return true;
	}

	int value0;
	if (!Parser_parse_integer(pr, &value0, accept_floor_notation))
		return false;

	Parser_skip_whitespaces(pr);

	// 1) n
	if (*pr->str != '-') {
		parse_result->type = RANGE_MATCHER_TYPE_EXACT;
		parse_result->value0 = value0;
		return true;
	}

	++pr->str; // skip '-'
	Parser_skip_whitespaces(pr);

	// 4) n - m
	if (*pr->str == '+' || *pr->str == '-' || isnum(*pr->str)) {
		int value1;
		if (!Parser_parse_integer(pr, &value1, accept_floor_notation))
			return false;

		if (value1 < value0) {
			int tmp = value0;
			value0 = value1;
			value1 = tmp;
		}

		parse_result->type = RANGE_MATCHER_TYPE_BETWEEN;
		parse_result->value0 = value0;
		parse_result->value1 = value1;
		return true;
	}

    // 3) n -
	if (!accept_open_range)
		return false;

	parse_result->type = RANGE_MATCHER_TYPE_GREATER;
	parse_result->value0 = value0;
	return true;
}

/**
 * 1) n
 * 2) - n
 * 3) n -
 * 4) n - m
 */
bool Parser_parse_range_matcher(Parser *pr, RangeMatcher *parse_result) {
	return Parser_parse_range_matcher_internal(
		pr,
		parse_result,
		RANGE_MATCHER_PARSE_MODE_ACCEPT_OPEN_RANGE,
		RANGE_MATCHER_PARSE_MODE_DENY_FLOOR_NOTATION);
}

/**
 * 1) n
 * 4) n - m
 */
bool Parser_parse_closed_range_matcher(Parser *pr, RangeMatcher *parse_result) {
	return Parser_parse_range_matcher_internal(
		pr,
		parse_result,
		RANGE_MATCHER_PARSE_MODE_DENY_OPEN_RANGE,
		RANGE_MATCHER_PARSE_MODE_DENY_FLOOR_NOTATION);
}

/**
 * 1) n       (closed)
 * 2) - n     (open)
 * 3) n -     (open)
 * 4) n - m   (closed)
 */
bool Parser_parse_floor_range_matcher(Parser *pr, RangeMatcher *parse_result) {
	return Parser_parse_range_matcher_internal(
		pr,
		parse_result,
		RANGE_MATCHER_PARSE_MODE_ACCEPT_OPEN_RANGE,
		RANGE_MATCHER_PARSE_MODE_ACCEPT_FLOOR_NOTATION);
}

bool Parser_parse_time(Parser *pr, int *parse_result) {
	// Time needs 5 characters, e.g., "00:00".
	for (size_t i = 0; i < 5; ++i) {
		if (!ptr->str[i]) {
			return false;
		}
	}

	const char h1 = ptr->str[0];
	const char h2 = ptr->str[1];
	const char colon = ptr->str[2];
	const char m1 = ptr->str[3];
	const char m2 = ptr->str[4];
	ptr->str += 5;

	if (!isnum(h1) || !isnum(h2) || colon != ':' || !isnum(m1) || !isnum(m2))
		return false;

	const auto h = 10 * (h1 - '0') + (h2 - '0');
	const auto m = 10 * (m1 - '0') + (m2 - '0');

	if (h < 0 || 23 < h || m < 0 || 59 < m)
		return false;

	*parse_result = 60 * h + m;
}

/**
 * 1) n       (closed)
 * 2) n - m   (closed)
 */
bool Parser_parse_time_range_matcher(Parser *pr, RangeMatcher *parse_result) {
	Parser_skip_whitespaces(pr);

	int value0;
	if (!Parser_parse_time(pr, value0))
		return false;

	Parser_skip_whitespaces(pr);

	// 1) n
	if (*pr->str != '-') {
		parse_result->type = RANGE_MATCHER_TYPE_EXACT;
		parse_result->value0 = value0;
		return true;
	}

	++pr->str; // skip '-'
	Parser_skip_whitespaces(pr);

	// 2) n - m
	int value1;
	if (!Parser_parse_time(pr, &value1))
		return false;

	if (value1 < value0) {
		int tmp = value0;
		value0 = value1;
		value1 = tmp;
	}

	parse_result->type = RANGE_MATCHER_TYPE_BETWEEN;
	parse_result->value0 = value0;
	parse_result->value1 = value1;
	return true;
}

bool RangeMatcher_compares(const RangeMatcher *rm, int value) {
	// assert(rm);

	switch (rm->type) {
	case RANGE_MATCHER_TYPE_EXACT:         return value == rm->value0;
	case RANGE_MATCHER_TYPE_LESS:          return value <= rm->value0;
	case RANGE_MATCHER_TYPE_GREATER:       return value >= rm->value0;
	case RANGE_MATCHER_TYPE_BETWEEN:       return rm->value0 <= value && value <= rm->value1;
	default: return false;
	}
}

bool Parser_parse_intcomparison_value(Parser *pr, IntComparisonValue *parse_result) {
	// TODO
}

bool Parser_parse_intcomparison_operator(Parser *pr, IntComparisonOperator *parse_result) {
	Parser_skip_whitespaces(pr);

	switch (*pr->str) {
	case '=':
		++pr->str;
		switch (*pr->str) {
		case '='
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_EQ;
			return true;
		case '<'
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_LE;
			return true;
		case '>'
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_GE;
			return true;
		default:
			return false;
		}
	case '<':
		++pr->str;
		switch (*pr->str) {
		case '='
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_LE;
			return true;
		case '*'
			++pr->str;
			Parser_skip_whitespaces(pr);
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_LE_MUL;
			{
				IntComparisonValue value;
				if (!Parser_parse_floating(pr, &value))
					return false;
				parse_result->extra = value;
			}
			return true;
		case '+'
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_LE_ADD;
			{
				IntComparisonValue value;
				if (!Parser_parse_floating(pr, &value))
					return false;
				parse_result->extra = value;
			}
			return true;
		default:
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_LT;
			return true;
		}
	case '>':
		++pr->str;
		switch (*pr->str) {
		case '='
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_GE;
			return true;
		case '*'
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_GE_MUL;
			{
				IntComparisonValue value;
				if (!Parser_parse_floating(pr, &value))
					return false;
				parse_result->extra = value;
			}
			return true;
		case '+'
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_GE_ADD;
			{
				IntComparisonValue value;
				if (!Parser_parse_floating(pr, &value))
					return false;
				parse_result->extra = value;
			}
			return true;
		default:
			++pr->str;
			parse_result->type = INTCOMPARISON_OPERATOR_TYPE_GT;
			return true;
		}
	default:
		return false;
	}
}

bool IntComparisonValue_compares(IntComparisonValue lhs, IntComparisonValue rhs, const IntComparisonOperator *op) {
	switch (op->type) {
	case INTCOMPARISON_OPERATOR_TYPE_EQ:     return lhs              ==  rhs;
	case INTCOMPARISON_OPERATOR_TYPE_LE:     return lhs              <=  rhs;
	case INTCOMPARISON_OPERATOR_TYPE_LT:     return lhs              <   rhs;
	case INTCOMPARISON_OPERATOR_TYPE_GE:     return lhs              >=  rhs;
	case INTCOMPARISON_OPERATOR_TYPE_GT:     return lhs              >   rhs;
	case INTCOMPARISON_OPERATOR_TYPE_LE_MUL: return lhs * op->extra  <=  rhs;
	case INTCOMPARISON_OPERATOR_TYPE_GE_MUL: return lhs              >=  rhs * op->extra;
	case INTCOMPARISON_OPERATOR_TYPE_LE_ADD: return lhs + op->extra  <=  rhs;
	case INTCOMPARISON_OPERATOR_TYPE_GE_ADD: return lhs              >=  rhs + op->extra;
	default: return false;
	}
}

bool Parser_parse_strcomparison_value(Parser *pr, StrComparisonValue *parse_result) {
	static char static_buffer[32];

	Parser_skip_whitespaces(pr);

	if (*pr->str == '"') {
		// TODO string literal
		return true;
	}

	Token tok;
	if (!Parser_parse_one_token_of(pr, &tok, TOKEN_TYPE_IDENTIFIER))
		return false;

	int chara_id = pr->data->unitid;
	if (tok.len >= 3 && (tok.str[0] == 'P' && tok.str[1] == 'C')) {
		chara_id = 0;
		tok.str += 2;
		tok.len -= 2;
	}

	switch (*tok.str) {
	case 'A':
		if (Token_equals_literal(&tok, "Aka")) {
			StrComparisonValue_set_string(parse_result, cdatan(1, chara_id));
			return true;
		}
		if (Token_equals_literal(&tok, "Age")) {
			StrComparisonValue_set_integer(parse_result, gdata(10) - cdata(21, chara_id));
			return true;
		}
		break;
	case 'C':
		if (Token_equals_literal(&tok, "Class")) {
			StrComparisonValue_set_string(parse_result, cdatan(3, chara_id));
			return true;
		}
		if (Token_equals_literal(&tok, "Cash")) {
			StrComparisonValue_set_integer(parse_result, cdata(30, chara_id));
			return true;
		}
		break;
	case 'D':
		if (Token_equals_literal(&tok, "Date")) {
			StrComparisonValue_set_integer(parse_result, gdata(12));
			return true;
		}
		break;
	case 'F':
		if (Token_equals_literal(&tok, "Fame")) {
			StrComparisonValue_set_integer(parse_result, cdata(34, chara_id));
			return true;
		}
		break;
	case 'H':
		if (Token_equals_literal(&tok, "Height")) {
			StrComparisonValue_set_integer(parse_result, cdata(19, chara_id));
			return true;
		}
		if (Token_equals_literal(&tok, "Hour")) {
			StrComparisonValue_set_integer(parse_result, gdata(13));
			return true;
		}
		break;
	case 'I':
		if (Token_equals_literal(&tok, "Impression")) {
			StrComparisonValue_set_integer(parse_result, cdata(17, chara_id));
			return true;
		}
		break;
	case 'K':
		if (Token_equals_literal(&tok, "Karma")) {
			StrComparisonValue_set_integer(parse_result, cdata(49, 0));
			return true;
		}
		break;
	case 'M':
		if (Token_equals_literal(&tok, "Month")) {
			StrComparisonValue_set_integer(parse_result, gdata(11));
			return true;
		}
		break;
	case 'N':
		if (Token_equals_literal(&tok, "Name")) {
			StrComparisonValue_set_string(parse_result, cdatan(0, chara_id));
			return true;
		}
		break;
	case 'R':
		if (Token_equals_literal(&tok, "Race")) {
			StrComparisonValue_set_string(parse_result, cdatan(2, chara_id));
			return true;
		}
		if (Token_equals_literal(&tok, "Religion")) {
			switch (cdata(61, chara_id)) {
			case 0:
				StrComparisonValue_set_string_literal(parse_result, "無のエイス");
				return true;
			case 1:
				StrComparisonValue_set_string_literal(parse_result, "機械のマニ");
				return true;
			case 2:
				StrComparisonValue_set_string_literal(parse_result, "風のルルウィ");
				return true;
			case 3:
				StrComparisonValue_set_string_literal(parse_result, "元素のイツパロトル");
				return true;
			case 4:
				StrComparisonValue_set_string_literal(parse_result, "幸運のエヘカトル");
				return true;
			case 5:
				StrComparisonValue_set_string_literal(parse_result, "地のオパートス");
				return true;
			case 6:
				StrComparisonValue_set_string_literal(parse_result, "癒しのジュア");
				return true;
			case 7:
				StrComparisonValue_set_string_literal(parse_result, "収穫のクミロミ");
				return true;
			default:
				StrComparisonValue_set_string_literal(parse_result, "-1");
				return true;
			}
		}
		break;
	case 'W':
		if (Token_equals_literal(&tok, "Weight")) {
			StrComparisonValue_set_integer(parse_result, cdata(20, chara_id));
			return true;
		}
		break;
	case 'Y':
		if (Token_equals_literal(&tok, "Year")) {
			StrComparisonValue_set_integer(parse_result, gdata(10));
			return true;
		}
		break;
	case 'n':
		if (Token_equals_literal(&tok, "nReligion")) {
			switch (cdata(61, chara_id)) {
			case 0:
				StrComparisonValue_set_string_literal(parse_result, "エイス");
				return true;
			case 1:
				StrComparisonValue_set_string_literal(parse_result, "マニ");
				return true;
			case 2:
				StrComparisonValue_set_string_literal(parse_result, "ルルウィ");
				return true;
			case 3:
				StrComparisonValue_set_string_literal(parse_result, "イツパロトル");
				return true;
			case 4:
				StrComparisonValue_set_string_literal(parse_result, "エヘカトル");
				return true;
			case 5:
				StrComparisonValue_set_string_literal(parse_result, "オパートス");
				return true;
			case 6:
				StrComparisonValue_set_string_literal(parse_result, "ジュア");
				return true;
			case 7:
				StrComparisonValue_set_string_literal(parse_result, "クミロミ");
				return true;
			default:
				StrComparisonValue_set_string_literal(parse_result, "-1");
				return true;
			}
		}
		break;
	default:
		break;
	}

	if (Token_starts_with_literal(&tok, "global:")) {
		tok.str += 7;
		tok.len -= 7;
		const char *value = getvalue(pr->data->addtgvar, &tok);
		if (*value) {
			StrComparisonValue_set_string(parse_result, value);
			return true;
		}
	} else {
		const char *value = getvalue(strarray(pr->data->hei, pr->data->addtlvarPVal, pr->data->unitid), &tok);
		if (*value) {
			StrComparisonValue_set_string(parse_result, value);
			return true;
		}
	}

	StrComparisonValue_set_string_literal(parse_result, "-1");
	return true;
}

bool StrComparisonValue_equals(const StrComparisonValue *lhs, const StrComparisonValue *rhs) {
	const size_t L = lhs->len, R = rhs->len;

	if (L != R)
		return false;

	for (size_t i = 0; i < L; ++i) {
		if (lhs->str[i] != rhs->str[i]) {
			return false;
		}
	}
	return true;
}

bool StrComparisonValue_contains(const StrComparisonValue *lhs, const StrComparisonValue *rhs) {
	const size_t L = lhs->len, R = rhs->len;

	if (L < R)
		return false;

	for (size_t start = 0; start <= L - R; ++start) {
		for (size_t i = 0; i < R; ++i) {
			if (lhs->str[i + start] != rhs->str[i]) {
				goto continue_outer_for;
			}
		}
		return true;
continue_outer_for:
	}

	return false;
}

/* キーに対応する値文字列を返す, キーが無ければ-1を返す
   static charへのポインタを返すので連続で呼び出すと前回の文字列は破棄される */
const char* getvalue(char *vp, char *key) {
	static char value[256];
	char keyc[259];
	strcpy(keyc, ";");
	strcat(keyc, key);
	strcat(keyc, ",");

	char *s = strstr(vp, keyc);
	int len = strlen(keyc);

	if (s != NULL) {
		int vlen = min(search(s + len, ';'), 255);
		strncpy(value, s + len, vlen);
		value[vlen] = '\0';
		return value;
	}
	else {
		return "-1";
	}
}

double rankattach(double value) {
	if (value < 50.0) {
		return 1.0;
	}
	if (50.0 <= value && value < 100.0) {
		return 2.0;
	}
	if (100.0 <= value && value < 150.0) {
		return 3.0;
	}
	if (150.0 <= value && value < 200.0) {
		return 4.0;
	}
	if (200.0 <= value) {
		return 5.0;
	}
	return 0.0;
}

int intarray(HSPEXINFO *hei, PVal *intarray, int index) {
	HspVarCoreReset(intarray);
	hei->HspFunc_array(intarray, index);
	return *(int *)hei->HspFunc_getproc(HSPVAR_FLAG_INT)->GetPtr(intarray);
}

int intmap(HSPEXINFO *hei, PVal *intmap, int index1, int index2) {
	HspVarCoreReset(intmap);
	hei->HspFunc_array(intmap, index1);
	hei->HspFunc_array(intmap, index2);
	return *(int *)hei->HspFunc_getproc(HSPVAR_FLAG_INT)->GetPtr(intmap);
}

char* strarray(HSPEXINFO *hei, PVal *strarray, int index) {
	HspVarCoreReset(strarray);
	hei->HspFunc_array(strarray, index);
	return (char *)hei->HspFunc_getproc(HSPVAR_FLAG_STR)->GetPtr(strarray);
}

char* strmap(HSPEXINFO *hei, PVal *strmap, int index1, int index2) {
	HspVarCoreReset(strmap);
	hei->HspFunc_array(strmap, index1);
	hei->HspFunc_array(strmap, index2);
	return (char *)hei->HspFunc_getproc(HSPVAR_FLAG_STR)->GetPtr(strmap);
}

bool cbit(args data, int unitid, int bitid) {
	int checker = 1;
	checker <<= bitid % 32;
	return ((checker & cdata(450 + (bitid / 32), unitid)) == 0) ? false : true;
}

double scomval(args data, int index, int unitid, bool potentialflag, bool expflag, bool revisionflag) {
	if (!potentialflag) {
		if (revisionflag) {
			return expflag ? double(sdata(index, unitid)) + double((sdata(index + 600, unitid) % 1000000 - sdata(index + 600, unitid) % 1000) / 1000 / 1000) : double(sdata(index, unitid));
		}
		else {
			return expflag ? double((sdata(index + 600, unitid) - sdata(index + 600, unitid) % 1000000) / 1000000) + double((sdata(index + 600, unitid) % 1000000 - sdata(index + 600, unitid) % 1000) / 1000 / 1000) : double((sdata(index + 600, unitid) - sdata(index + 600, unitid) % 1000000) / 1000000);
		}
	}
	else {
		return double(sdata(index + 600, unitid) % 1000);
	}
	return 0.0;
}

double comparisonval(args data, char *target) {
	bool potentialflag = false, rankflag = false, expflag = false, revisionflag = false;
	int sdataindex = 0, referid = data.unitid;

	if (target[0] == 'P' && target[1] == 'C') {
		referid = 0;
		target += 2;
	}
	for (int i = strlen(target) - 1; i != 0; i--) {
		if (target[i] == 'P') {
			potentialflag = true;
		}
		else if (target[i] == 'R') {
			rankflag = true;
		}
		else if (target[i] == 'E') {
			expflag = true;
		}
		else if (target[i] == '+') {
			revisionflag = true;
		}
		else {
			target[i + 1] = '\0';
			break;
		}
	}

	if (decimalonly(target)) {
		return rattach(atof(target));
	}

	switch (target[0]) {
	case 'A':
		if (equals(target, "Anatomy")) {
			return rattach(scomval(data, 161, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Alchemy")) {
			return rattach(scomval(data, 178, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Axe")) {
			return rattach(scomval(data, 102, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Age")) {
			return rattach((double)(gdata(10) - cdata(21, referid)));
		}
		break;
	case 'B':
		if (equals(target, "Blunt")) {
			return rattach(scomval(data, 103, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Bow")) {
			return rattach(scomval(data, 108, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'C':
		if (equals(target, "Constitution")) {
			return rattach(scomval(data, 11, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Charm")) {
			return rattach(scomval(data, 17, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Carpentry")) {
			return rattach(scomval(data, 176, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Cooking")) {
			return rattach(scomval(data, 184, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Casting")) {
			return rattach(scomval(data, 172, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Control_Magic")) {
			return rattach(scomval(data, 188, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Crossbow")) {
			return rattach(scomval(data, 109, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'D':
		if (equals(target, "Dexterity")) {
			return rattach(scomval(data, 12, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Dual_Wield")) {
			return rattach(scomval(data, 166, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Disarm_Trap")) {
			return rattach(scomval(data, 175, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Detection")) {
			return rattach(scomval(data, 159, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'E':
		if (equals(target, "Evasion")) {
			return rattach(scomval(data, 173, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Eye_of_Mind")) {
			return rattach(scomval(data, 186, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'F':
		if (equals(target, "Fishing")) {
			return rattach(scomval(data, 185, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Faith")) {
			return rattach(scomval(data, 181, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Firearm")) {
			return rattach(scomval(data, 110, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Fame")) {
			return rattach((double)cdata(34, referid));
		}
		break;
	case 'G':
		if (equals(target, "Greater_Evasion")) {
			return rattach(scomval(data, 187, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Gene_engineer")) {
			return rattach(scomval(data, 151, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Gardening")) {
			return rattach(scomval(data, 180, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'H':
		if (equals(target, "Healing")) {
			return rattach(scomval(data, 154, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Heavy_Armor")) {
			return rattach(scomval(data, 169, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Hp")) {
			return rattach((double)cdata(50, referid));
		}
		if (equals(target, "Height")) {
			return rattach((double)cdata(19, referid));
		}
		break;
	case 'I':
		if (equals(target, "Investing")) {
			return rattach(scomval(data, 160, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'J':
		if (equals(target, "Jeweler")) {
			return rattach(scomval(data, 179, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'L':
		if (equals(target, "Learning")) {
			return rattach(scomval(data, 14, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Lack")) {
			return rattach(scomval(data, 19, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Life")) {
			return rattach(scomval(data, 2, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Light_Armor")) {
			return rattach(scomval(data, 171, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Lock_Picking")) {
			return rattach(scomval(data, 158, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Literacy")) {
			return rattach(scomval(data, 150, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Long_Sword")) {
			return rattach(scomval(data, 100, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Level")) {
			return rattach((double)cdata(38, referid));
		}
		break;
	case 'M':
		switch (target[1]) {
		case 'a':
			if (equals(target, "Magic")) {
				return rattach(scomval(data, 16, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Mana")) {
				return rattach(scomval(data, 3, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "MaxHp")) {
				return rattach((double)cdata(51, referid));
			}
			if (equals(target, "MaxSp")) {
				return rattach((double)cdata(53, referid));
			}
			if (equals(target, "MaxMp")) {
				return rattach((double)cdata(56, referid));
			}
			if (equals(target, "Marksman")) {
				return rattach(scomval(data, 189, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Magic_Capacity")) {
				return rattach(scomval(data, 164, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Magic_Device")) {
				return rattach(scomval(data, 174, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Martial_Arts")) {
				return rattach(scomval(data, 106, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'e':
			if (equals(target, "Medium_Armor")) {
				return rattach(scomval(data, 170, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Memorization")) {
				return rattach(scomval(data, 165, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Meditation")) {
				return rattach(scomval(data, 155, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'i':
			if (equals(target, "Mining")) {
				return rattach(scomval(data, 163, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'p':
			if (equals(target, "Mp")) {
				return rattach((double)cdata(55, referid));
			}
			break;
		default:
			break;
		}
		break;
	case 'N':
		if (equals(target, "Negotiation")) {
			return rattach(scomval(data, 156, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'P':
		if (equals(target, "Perception")) {
			return rattach(scomval(data, 13, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Pickpocket")) {
			return rattach(scomval(data, 300, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Performer")) {
			return rattach(scomval(data, 183, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Polearm")) {
			return rattach(scomval(data, 104, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'R':
		if (equals(target, "Riding")) {
			return rattach(scomval(data, 301, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'S':
		switch (target[1]) {
		case 'a':
			if (equals(target, "Sanity")) {
				return rattach((double)cdata(34, referid));
			}
			break;
		case 'c':
			if (equals(target, "Scythe")) {
				return rattach(scomval(data, 107, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'e':
			if (equals(target, "Sense_Quality")) {
				return rattach(scomval(data, 162, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'h':
			if (equals(target, "Shield")) {
				return rattach(scomval(data, 168, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Short_Sword")) {
				return rattach(scomval(data, 101, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'p':
			if (equals(target, "Speed")) {
				return rattach(scomval(data, 18, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Sp")) {
				return rattach((double)cdata(52, referid));
			}
			break;
		case 't':
			if (equals(target, "Strength")) {
				return rattach(scomval(data, 10, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Stealth")) {
				return rattach(scomval(data, 157, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Stave")) {
				return rattach(scomval(data, 105, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'u':
			if (equals(target, "Superb")) {
				return rattach(5.0);
			}
			break;
		default:
			break;
		}
		break;
	case 'T':
		if (equals(target, "Tactics")) {
			return rattach(scomval(data, 152, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Two_Hand")) {
			return rattach(scomval(data, 167, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Tailoring")) {
			return rattach(scomval(data, 177, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Traveling")) {
			return rattach(scomval(data, 182, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Throwing")) {
			return rattach(scomval(data, 111, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'W':
		if (equals(target, "Willpower")) {
			return rattach(scomval(data, 15, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Weight_Lifting")) {
			return rattach(scomval(data, 153, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Weight")) {
			return rattach((double)cdata(20, referid));
		}
		break;
	case 'b':
		if (equals(target, "bad")) {
			return rattach(2.0);
		}
		break;
	case 'c':
		if (equals(target, "cold")) {
			return rattach(scomval(data, 51, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "chaos")) {
			return rattach(scomval(data, 59, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "cut")) {
			return rattach(scomval(data, 61, referid, potentialflag, expflag, revisionflag));
		}
		if (strstr(target, "cdata") == target) {
			target += 5;
			return rattach((double)cdata(atoi(target), data.unitid));
		}
		if (strstr(target, "ccdata") == target) {
			target += 6;
			return rattach((double)cdata(atoi(target), data.ccid));
		}
		if (strstr(target, "csdata") == target) {
			target += 6;
			return rattach((double)sdata(atoi(target), data.ccid));
		}
		break;
	case 'd':
		if (equals(target, "darkness")) {
			return rattach(scomval(data, 53, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'f':
		if (equals(target, "fire")) {
			return rattach(scomval(data, 50, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'g':
		if (strstr(target, "gdata") == target) {
			target += 5;
			return rattach((double)gdata(atoi(target)));
		}
		if (equals(target, "great")) {
			return rattach(4.0);
		}
		if (equals(target, "good")) {
			return rattach(3.0);
		}
		break;
	case 'h':
		if (equals(target, "hopeless")) {
			return rattach(1.0);
		}
		break;
	case 'l':
		if (equals(target, "lightning")) {
			return rattach(scomval(data, 52, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'm':
		if (equals(target, "mind")) {
			return rattach(scomval(data, 54, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "magic")) {
			return rattach(scomval(data, 60, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'n':
		if (equals(target, "nether")) {
			return rattach(scomval(data, 56, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "nerve")) {
			return rattach(scomval(data, 58, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'p':
		if (equals(target, "poison")) {
			return rattach(scomval(data, 55, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 's':
		if (equals(target, "sound")) {
			return rattach(scomval(data, 57, referid, potentialflag, expflag, revisionflag));
		}
		if (strstr(target, "sdata") == target) {
			target += 5;
			return rattach((double)sdata(atoi(target), data.unitid));
		}
		break;
	case 't':
		if (strstr(target, "tcdata") == target) {
			target += 6;
			return rattach((double)cdata(atoi(target), data.tcid));
		}
		if (strstr(target, "tsdata") == target) {
			target += 6;
			return rattach((double)sdata(atoi(target), data.tcid));
		}
		break;
	default:
		break;
	}

	if (strstr(target, "global:") == target) {
		target += 7;
		return atof(getvalue(data.addtgvar, target));
	}
	return atof(getvalue(strarray(data.hei, data.addtlvarPVal, referid), target));
}

void Parser_init(Parser *pr, const char* condition, args *data) {
	pr->str = condition;
	pr->data = data;
}

void Parser_skip_whitespaces(Parser *pr) {
	for (; isspace(*pr->str); ++pr->str)
		;
}

bool Parser_parse_primary_expression(Parser *pr) {
	if (pr->str[strlen(pr->str) - 1] == '\r') {
		pr->str[strlen(pr->str) - 1] = '\0';
	}

	Token tok;
	if (!Parser_parse_one_token(pr, &tok))
		return false;

	if (tok.type == TOKEN_TYPE_LEFT_PAREN) {
		// nested expression
		bool result = Parser_parse_expression(pr);
		Parser_skip_whitespaces(pr);
		if (*pr->str == ')') {
			++pr->str;
			return result;
		} else {
			return false;
		}
	}

	if (tok.type != TOKEN_TYPE_IDENTIFIER)
		return false;

	switch (tok.str[0]) {
	case 'P':
		switch (tok.str[2]) {
		case 'a':
			if (Token_equals_literal(&tok, "PCaction")) {
				Token arg_0;
				if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
					return false;

				if (Token_equals_literal(&arg_0, "Performance")) {
					return cdata(140, 0) == 6;
				}
				if (Token_equals_literal(&arg_0, "Dig")) {
					return cdata(140, 0) == 5;
				}
				if (Token_equals_literal(&arg_0, "Reading")) {
					return cdata(140, 0) == 2;
				}
				if (Token_equals_literal(&arg_0, "Fishing")) {
					return cdata(140, 0) == 7;
				}
				if (Token_equals_literal(&arg_0, "Harvesting")) {
					return cdata(140, 0) == 8;
				}
				if (Token_equals_literal(&arg_0, "Search")) {
					return cdata(140, 0) == 9;
				}
				if (Token_equals_literal(&arg_0, "Eat")) {
					return cdata(140, 0) == 1;
				}
				return false;
			}
			break;
		case 'c':
			if (Token_equals_literal(&tok, "PCcondition")) {
				Token arg_0;
				if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
					return false;

				if (Token_equals_literal(&arg_0, "Normal")) {
					return cdata(250, 0) == 0 && cdata(251, 0) == 0 && cdata(252, 0) == 0 && cdata(253, 0) == 0 && cdata(254, 0) == 0 && cdata(255, 0) == 0 && cdata(256, 0) == 0 && cdata(257, 0) == 0 && cdata(258, 0) == 0 && cdata(259, 0) == 0 && cdata(260, 0) == 0 && cdata(261, 0) == 0 && cdata(264, 0) == 0;
				}
				if (Token_equals_literal(&arg_0, "Poisoned")) {
					return 0 < cdata(250, 0);
				}
				if (Token_equals_literal(&arg_0, "Sleep")) {
					return 0 < cdata(251, 0);
				}
				if (Token_equals_literal(&arg_0, "Paralyzed")) {
					return 0 < cdata(252, 0);
				}
				if (Token_equals_literal(&arg_0, "Blinded")) {
					return 0 < cdata(253, 0);
				}
				if (Token_equals_literal(&arg_0, "Confused")) {
					return 0 < cdata(254, 0);
				}
				if (Token_equals_literal(&arg_0, "Fear")) {
					return 0 < cdata(255, 0);
				}
				if (Token_equals_literal(&arg_0, "Dim")) {
					return 0 < cdata(256, 0);
				}
				if (Token_equals_literal(&arg_0, "Drunk")) {
					return 0 < cdata(257, 0);
				}
				if (Token_equals_literal(&arg_0, "Bleeding")) {
					return 0 < cdata(258, 0);
				}
				if (Token_equals_literal(&arg_0, "Wet")) {
					return 0 < cdata(259, 0);
				}
				if (Token_equals_literal(&arg_0, "Insane")) {
					return 0 < cdata(260, 0);
				}
				if (Token_equals_literal(&arg_0, "Sick")) {
					return 0 < cdata(261, 0);
				}
				if (Token_equals_literal(&arg_0, "Fury")) {
					return 0 < cdata(264, 0);
				}
				if (Token_equals_literal(&arg_0, "sleepiness")) {
					RangeMatcher rm;
					if (!Parser_parse_range_matcher(pr, &rm))
						return false;

					return RangeMatcher_compares(&rm, gdata(90));
				}
			}
			if (Token_equals_literal(&tok, "PCcash")) {
				RangeMatcher rm;
				if (!Parser_parse_range_matcher(pr, &rm))
					return false;

				return RangeMatcher_compares(&rm, cdata(30, 0));
			}
			if (Token_equals_literal(&tok, "PCclass")) {
				Token arg_0;
				if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
					return false;

				if (Token_equals_literal(&arg_0, "same")) {
					return equals(cdatan(3, pr->data->unitid), cdatan(3, 0));
				} else {
					return Token_equals(&arg_0, cdatan(3, 0));
				}
			}
			break;
		case 'f':
			if (Token_equals_literal(&tok, "PCfame")) {
				RangeMatcher rm;
				if (!Parser_parse_range_matcher(pr, &rm))
					return false;

				return RangeMatcher_compares(&rm, cdata(34, 0));
			}
			break;
		case 'r':
			if (Token_equals_literal(&tok, "PCreligion")) {
				Token arg_0;
				if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
					return false;

				if (Token_equals_literal(&arg_0, "same")) {
					return cdata(61, pr->data->unitid) == cdata(61, 0);
				}
				if (Token_equals_literal(&arg_0, "Eyth")) {
					return cdata(61, 0) == 0;
				}
				if (Token_equals_literal(&arg_0, "Mani")) {
					return cdata(61, 0) == 1;
				}
				if (Token_equals_literal(&arg_0, "Lulwy")) {
					return cdata(61, 0) == 2;
				}
				if (Token_equals_literal(&arg_0, "Itzpalt")) {
					return cdata(61, 0) == 3;
				}
				if (Token_equals_literal(&arg_0, "Ehekatl")) {
					return cdata(61, 0) == 4;
				}
				if (Token_equals_literal(&arg_0, "Opatos")) {
					return cdata(61, 0) == 5;
				}
				if (Token_equals_literal(&arg_0, "Jure")) {
					return cdata(61, 0) == 6;
				}
				if (Token_equals_literal(&arg_0, "Kumiromi")) {
					return cdata(61, 0) == 7;
				}
				return false;
			}
			if (Token_equals_literal(&tok, "PCrace")) {
				Token arg_0;
				if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
					return false;

				if (Token_equals_literal(&arg_0, "same")) {
					return equals(cdatan(2, pr->data->unitid), cdatan(2, 0));
				} else {
					return Token_equals(&arg_0, cdatan(2, 0));
				}
			}
			break;
		case 's':
			if (Token_equals_literal(&tok, "PCsex")) {
				Token arg_0;
				if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
					return false;

				if (Token_equals_literal(&arg_0, "same")) {
					return cdata(8, pr->data->unitid) == cdata(8, 0);
				}
				if (Token_equals_literal(&arg_0, "Male")) {
					return cdata(8, 0) == 0;
				}
				if (Token_equals_literal(&arg_0, "Female")) {
					return cdata(8, 0) == 1;
				}
				return false;
			}
			break;
		default:
			break;
		}
		break;
	case 'a':
		if (Token_equals_literal(&tok, "action")) {
			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;

			if (Token_equals_literal(&arg_0, "Performance")) {
				return cdata(140, pr->data->unitid) == 6;
			}
			if (Token_equals_literal(&arg_0, "Eat")) {
				return cdata(140, pr->data->unitid) == 1;
			}
			return false;
		}
		if (Token_equals_literal(&tok, "agreement")) {
			return cbit(data, pr->data->unitid, 969);
		}
		if (Token_equals_literal(&tok, "anorexia")) {
			return cbit(data, pr->data->unitid, 986);
		}
		break;
	case 'c':
		if (Token_equals_literal(&tok, "condition")) {
			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;

			if (Token_equals_literal(&arg_0, "Normal")) {
				return cdata(250, pr->data->unitid) == 0 &&
					cdata(251, pr->data->unitid) == 0 &&
					cdata(252, pr->data->unitid) == 0 &&
					cdata(253, pr->data->unitid) == 0 &&
					cdata(254, pr->data->unitid) == 0 &&
					cdata(255, pr->data->unitid) == 0 &&
					cdata(256, pr->data->unitid) == 0 &&
					cdata(257, pr->data->unitid) == 0 &&
					cdata(258, pr->data->unitid) == 0 &&
					cdata(259, pr->data->unitid) == 0 &&
					cdata(260, pr->data->unitid) == 0 &&
					cdata(261, pr->data->unitid) == 0 &&
					cdata(264, pr->data->unitid) == 0;
			}
			if (Token_equals_literal(&arg_0, "Poisoned")) {
				return 0 < cdata(250, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Sleep")) {
				return 0 < cdata(251, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Paralyzed")) {
				return 0 < cdata(252, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Blinded")) {
				return 0 < cdata(253, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Confused")) {
				return 0 < cdata(254, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Fear")) {
				return 0 < cdata(255, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Dim")) {
				return 0 < cdata(256, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Drunk")) {
				return 0 < cdata(257, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Bleeding")) {
				return 0 < cdata(258, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Wet")) {
				return 0 < cdata(259, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Insane")) {
				return 0 < cdata(260, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Sick")) {
				return 0 < cdata(261, pr->data->unitid);
			}
			if (Token_equals_literal(&arg_0, "Fury")) {
				return 0 < cdata(264, pr->data->unitid);
			}
		}
		if (Token_equals_literal(&tok, "cash")) {
			RangeMatcher rm;
			if (!Parser_parse_range_matcher(pr, &rm))
				return false;

			return RangeMatcher_compares(&rm, cdata(30, pr->data->unitid));
		}
		if (Token_equals_literal(&tok, "class")) {
			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;

			if (Token_equals_literal(&arg_0, "same")) {
				return equals(cdatan(3, pr->data->unitid), cdatan(3, 0));
			}
			else {
				return Token_equals(&arg_0, cdatan(3, pr->data->unitid));
			}
		}
		if (Token_equals_literal(&tok, "comparison")) {
			IntComparisonValue lhs;
			if (!Parser_parse_intcomparison_value(pr, &lhs))
				return false;

			IntComparisonOperator op;
			if (!Parser_parse_intcomparison_operator(pr, &op))
				return false;

			IntComparisonValue rhs;
			if (!Parser_parse_intcomparison_value(pr, &rhs))
				return false;

			return IntComparisonValue_compares(&lhs, &rhs, &op);
		}
		break;
	case 'f':
		if (Token_equals_literal(&tok, "fame")) {
			RangeMatcher rm;
			if (!Parser_parse_range_matcher(pr, &rm))
				return false;

			return RangeMatcher_compares(&rm, cdata(34, pr->data->unitid));
		}
		if (Token_equals_literal(&tok, "false")) {
			return false;
		}
		break;
	case 'i':
		if (Token_equals_literal(&tok, "impression")) {
			RangeMatcher rm = Parser_parse_range_matcher(pr);
			if (rm.type != RANGE_MATCHER_TYPE_ERROR) {

				return RangeMatcher_compares(&rm, cdata(17, pr->data->unitid));
			}

			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;

			if (Token_equals_literal(&arg_0, "Foe")) {
				return cdata(17, pr->data->unitid) <= 9;
			}
			if (Token_equals_literal(&arg_0, "Hate")) {
				return 10 <= cdata(17, pr->data->unitid) && cdata(17, pr->data->unitid) <= 24;
			}
			if (Token_equals_literal(&arg_0, "Annoying")) {
				return 25 <= cdata(17, pr->data->unitid) && cdata(17, pr->data->unitid) <= 39;
			}
			if (Token_equals_literal(&arg_0, "Normal")) {
				return 40 <= cdata(17, pr->data->unitid) && cdata(17, pr->data->unitid) <= 74;
			}
			if (Token_equals_literal(&arg_0, "Amiable")) {
				return 75 <= cdata(17, pr->data->unitid) && cdata(17, pr->data->unitid) <= 99;
			}
			if (Token_equals_literal(&arg_0, "Friend")) {
				return 100 <= cdata(17, pr->data->unitid) && cdata(17, pr->data->unitid) <= 149;
			}
			if (Token_equals_literal(&arg_0, "Fellow")) {
				return 150 <= cdata(17, pr->data->unitid) && cdata(17, pr->data->unitid) <= 199;
			}
			if (Token_equals_literal(&arg_0, "Soul_Mate")) {
				return 200 <= cdata(17, pr->data->unitid) && cdata(17, pr->data->unitid) <= 299;
			}
			if (Token_equals_literal(&arg_0, "Love")) {
				return cdata(17, pr->data->unitid) >= 300;
			}
			return false;
		}
		if (Token_equals_literal(&tok, "incognito")) {
			return cbit(data, pr->data->unitid, 16);
		}
		break;
	case 'k':
		if (Token_equals_literal(&tok, "karma")) {
			RangeMatcher rm;
			if (!Parser_parse_range_matcher(pr, &rm))
				return false;

			return RangeMatcher_compares(&rm, cdata(49, 0));
		}
		break;
	case 'l':
		if (Token_equals_literal(&tok, "layhand")) {
			return cbit(data, pr->data->unitid, 974);
		}
		break;
	case 'm':
		if (Token_equals_literal(&tok, "married")) {
			return cbit(data, pr->data->unitid, 961);
		}
		break;
	case 'p':
		if (Token_equals_literal(&tok, "pet")) {
			return cdata(9, pr->data->unitid) == 10;
		}
		break;
	case 'r':
		if (Token_equals_literal(&tok, "religion")) {
			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;
			if (Token_equals_literal(&arg_0, "same")) {
				return cdata(61, pr->data->unitid) == cdata(61, 0);
			}
			if (Token_equals_literal(&arg_0, "Eyth")) {
				return cdata(61, pr->data->unitid) == 0;
			}
			if (Token_equals_literal(&arg_0, "Mani")) {
				return cdata(61, pr->data->unitid) == 1;
			}
			if (Token_equals_literal(&arg_0, "Lulwy")) {
				return cdata(61, pr->data->unitid) == 2;
			}
			if (Token_equals_literal(&arg_0, "Itzpalt")) {
				return cdata(61, pr->data->unitid) == 3;
			}
			if (Token_equals_literal(&arg_0, "Ehekatl")) {
				return cdata(61, pr->data->unitid) == 4;
			}
			if (Token_equals_literal(&arg_0, "Opatos")) {
				return cdata(61, pr->data->unitid) == 5;
			}
			if (Token_equals_literal(&arg_0, "Jure")) {
				return cdata(61, pr->data->unitid) == 6;
			}
			if (Token_equals_literal(&arg_0, "Kumiromi")) {
				return cdata(61, pr->data->unitid) == 7;
			}
			return false;
		}
		if (Token_equals_literal(&tok, "race")) {
			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;

			if (Token_equals_literal(&arg_0, "same")) {
				return equals(cdatan(2, pr->data->unitid), cdatan(2, 0));
			}
			else {
				return Token_equals(&arg_0, cdatan(2, pr->data->unitid));
			}
		}
		if (Token_equals_literal(&tok, "random")) {
			Token arg_0;
			if (!Parser_parse_one_token_of(pr, &arg_0, TOKEN_TYPE_INTEGER))
				return false;

			srand(GetTickCount());
			return rand() % 100 < atoi(tok.str);
		}
		if (Token_equals_literal(&tok, "ridden")) {
			return cbit(data, pr->data->unitid, 975);
		}
		break;
	case 's':
		if (Token_equals_literal(&tok, "sex")) {
			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;

			if (Token_equals_literal(&arg_0, "same")) {
				return cdata(8, pr->data->unitid) == cdata(8, 0);
			}
			if (Token_equals_literal(&arg_0, "Male")) {
				return cdata(8, pr->data->unitid) == 0;
			}
			if (Token_equals_literal(&arg_0, "Female")) {
				return cdata(8, pr->data->unitid) == 1;
			}
			return false;
		}
		if (Token_equals_literal(&tok, "stethoscope")) {
			return cbit(data, pr->data->unitid, 966);
		}
		if (Token_equals_literal(&tok, "strcomparison")) {
			StrComparisonValue lhs;
			if (!Parser_parse_strcomparison_value(pr, &lhs))
				return false;

			Token op;
			if (!Parser_parse_one_identifier(pr, &op, TOKEN_TYPE_IDENTIFIER))
				return false;

			StrComparisonValue rhs;
			if (!Parser_parse_strcomparison_value(pr, &rhs))
				return false;

			if (Token_equals_literal(&op, "equal")) {
				return StrComparisonValue_equals(&lhs, &rhs);
			}
			if (Token_equals_literal(&op, "instr")) {
				return StrComparisonValue_contains(&lhs, &rhs);
			}
			return false;
		}
		break;
	case 't':
		if (Token_equals_literal(&tok, "tied")) {
			return cbit(data, pr->data->unitid, 968);
		}
		if (Token_equals_literal(&tok, "true")) {
			return true;
		}
		break;
	case 'w':
		if (Token_equals_literal(&tok, "when")) {
			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;

			if (Token_equals_literal(&arg_0, "year")) {
				RangeMatcher rm;
				if (!Parser_parse_range_matcher(pr, &rm))
					return false;

				return RangeMatcher_compares(&rm, gdata(10));
			}
			if (Token_equals_literal(&arg_0, "month")) {
				RangeMatcher rm;
				if (!Parser_parse_range_matcher(pr, &rm))
					return false;

				return RangeMatcher_compares(&rm, gdata(11));
			}
			if (Token_equals_literal(&arg_0, "date")) {
				RangeMatcher rm;
				if (!Parser_parse_range_matcher(pr, &rm))
					return false;

				return RangeMatcher_compares(&rm, gdata(12));
			}
			if (Token_equals_literal(&arg_0, "hour")) {
				RangeMatcher rm;
				if (!Parser_parse_range_matcher(pr, &rm))
					return false;

				return RangeMatcher_compares(&rm, gdata(13));
			}
			if (Token_equals_literal(&arg_0, "time")) {
				RangeMatcher rm = parse_time_range_matcher(pr->str);
				if (rm.type == RANGE_MATCHER_TYPE_ERROR)
					return false;

				return RangeMatcher_compares(&rm, gdata(13) * 24 + gdata(14));
			}
			if (Token_equals_literal(&arg_0, "Midnight")) {
				return 0 <= gdata(13) && gdata(13) <= 3;
			}
			if (Token_equals_literal(&arg_0, "Dawn")) {
				return 4 <= gdata(13) && gdata(13) <= 7;
			}
			if (Token_equals_literal(&arg_0, "Morning")) {
				return 8 <= gdata(13) && gdata(13) <= 11;
			}
			if (Token_equals_literal(&arg_0, "Noon")) {
				return 12 <= gdata(13) && gdata(13) <= 15;
			}
			if (Token_equals_literal(&arg_0, "Dusk")) {
				return 16 <= gdata(13) && gdata(13) <= 19;
			}
			if (Token_equals_literal(&arg_0, "Night")) {
				return 20 <= gdata(13) && gdata(13) <= 23;
			}
			return false;
		}
		if (Token_equals_literal(&tok, "where")) {

			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;

			if (Token_equals_literal(&arg_0, "North_Tyris")) {
				return gdata(20) == 4;
			}
			if (Token_equals_literal(&arg_0, "Home")) {
				return gdata(20) == 7;
			}
			if (Token_equals_literal(&arg_0, "Party")) {
				return gdata(20) == 13 && gdata(70) == 1009;
			}
			if (Token_equals_literal(&arg_0, "Outskirts")) {
				return gdata(20) == 13 && gdata(70) == 1001;
			}
			if (Token_equals_literal(&arg_0, "Urban_Area")) {
				return gdata(20) == 13 && (gdata(70) == 1008 || gdata(70) == 1010);
			}
			if (Token_equals_literal(&arg_0, "Crop")) {
				return gdata(20) == 13 && gdata(70) == 1006;
			}
			if (Token_equals_literal(&arg_0, "Town")) {
				return gdata(20) == 5 || gdata(20) == 11 || gdata(20) == 12 || gdata(20) == 14 || gdata(20) == 15 || gdata(20) == 36 || gdata(20) == 33 || gdata(20) == 25;
			}
			if (Token_equals_literal(&arg_0, "Vernis")) {
				return gdata(20) == 5;
			}
			if (Token_equals_literal(&arg_0, "Port_Kapul")) {
				return gdata(20) == 11;
			}
			if (Token_equals_literal(&arg_0, "Yowyn")) {
				return gdata(20) == 12;
			}
			if (Token_equals_literal(&arg_0, "Derphy")) {
				return gdata(20) == 14;
			}
			if (Token_equals_literal(&arg_0, "Palmia")) {
				return gdata(20) == 15;
			}
			if (Token_equals_literal(&arg_0, "Lumiest")) {
				return gdata(20) == 36;
			}
			if (Token_equals_literal(&arg_0, "Noyel")) {
				return gdata(20) == 33;
			}
			if (Token_equals_literal(&arg_0, "Larna")) {
				return gdata(20) == 25;
			}
			if (Token_equals_literal(&arg_0, "Cyber_Dome")) {
				return gdata(20) == 21;
			}
			if (Token_equals_literal(&arg_0, "Pet_Arena")) {
				return gdata(20) == 40;
			}
			if (Token_equals_literal(&arg_0, "Truce_Ground")) {
				return gdata(20) == 20;
			}
			if (Token_equals_literal(&arg_0, "Graveyard")) {
				return gdata(20) == 10;
			}
			if (Token_equals_literal(&arg_0, "Embassy")) {
				return gdata(20) == 32;
			}
			if (Token_equals_literal(&arg_0, "Museum")) {
				return gdata(20) == 101;
			}
			if (Token_equals_literal(&arg_0, "Shop")) {
				return gdata(20) == 102;
			}
			if (Token_equals_literal(&arg_0, "Storage_House")) {
				return gdata(20) == 104;
			}
			if (Token_equals_literal(&arg_0, "Ranch")) {
				return gdata(20) == 31;
			}
			if (Token_equals_literal(&arg_0, "Shelter")) {
				return gdata(20) == 30;
			}
			if (Token_equals_literal(&arg_0, "Sister")) {
				return gdata(20) == 29;
			}
			if (Token_equals_literal(&arg_0, "Pyramid")) {
				return gdata(20) == 37;
			}
			if (Token_equals_literal(&arg_0, "Jail")) {
				return gdata(20) == 41;
			}
			if (Token_equals_literal(&arg_0, "Mountain_Pass")) {
				return gdata(20) == 26;
			}
			if (Token_equals_literal(&arg_0, "Wilderness")) {
				return gdata(20) == 2;
			}
			if (Token_equals_literal(&arg_0, "Show_House")) {
				return gdata(20) == 35;
			}
			if (Token_equals_literal(&arg_0, "Lesimas")) {
				return gdata(20) == 3;
			}
			if (Token_equals_literal(&arg_0, "Void")) {
				return gdata(20) == 42;
			}
			if (Token_equals_literal(&arg_0, "Nefia")) {
				return 450 <= gdata(20) && gdata(20) <= 500;
			}
			if (Token_equals_literal(&arg_0, "floor")) {
				int floor = 0;
				if (gdata(20) == 7 && gdata(22) != 1) {
					floor = (gdata(22) > 1) ? -(gdata(22) - 1) : -(gdata(22) - 2);
				}
				if ((adata(0, gdata(20)) != 3) && (adata(16, gdata(20)) == 3 || adata(16, gdata(20)) == 8 || adata(16, gdata(20)) == 13 || (mdata(6) >= 20 && mdata(6) <= 23) == 1))  {
					floor = gdata(22) - adata(17, gdata(20)) + 1;
				}

				RangeMatcher rm;
				if (!Parser_parse_floor_range_matcher(pr, &rm))
					return false;

				return RangeMatcher_compares(&rm, floor);
			}
			return false;
		}
		if (Token_equals_literal(&tok, "weather")) {
			Token arg_0;
			if (!Parser_parse_one_identifier(pr, &arg_0, TOKEN_TYPE_IDENTIFIER))
				return false;

			if (Token_equals_literal(&arg_0, "Sun")) {
				return gdata(17) == 0;
			}
			if (Token_equals_literal(&arg_0, "Rain")) {
				return gdata(17) == 3;
			}
			if (Token_equals_literal(&arg_0, "Snow")) {
				return gdata(17) == 2;
			}
			if (Token_equals_literal(&arg_0, "Hard_rain")) {
				return gdata(17) == 4;
			}
			if (Token_equals_literal(&arg_0, "Etherwind")) {
				return gdata(17) == 1;
			}
			return false;
		}
		break;
	default:
		break;
	}
	return false;
}

bool Parser_parse_prefix_expression(Parser *pr) {
	bool has_not_operator = *pr->str == '!';
	if (has_not_operator)
		++pr->str;

	bool result = Parser_parse_primary_expression(pr);
	return has_not_operator ? !result : result;
}

bool Parser_parse_or_expression_vbar(Parser *pr) {
	if (Parser_parse_not_expression(pr))
		return true;

	while (1) {
		Parser_skip_whitespaces(pr);
		if (*pr->str != '|')
			break;

		++pr->str; // skip '|' operator
		if (Parser_parse_not_expression(pr))
			return true;
	}

	return false;
}

// AND
bool Parser_parse_and_expression(Parser *pr) {
	if (!Parser_parse_or_expression_vbar(pr))
		return false;

	while (1) {
		Parser_skip_whitespaces(pr);
		if (*pr->str != ',')
			break;

		++pr->str; // skip ',' operator
		if (!Parser_parse_or_expression_vbar(pr))
			return false;
	}

	return true;
}

// OR
bool Parser_parse_or_expression_slash(Parser *pr) {
	if (Parser_parse_and_expression(pr))
		return true;

	while (1) {
		Parser_skip_whitespaces(pr);
		if (*pr->str != '/')
			break;

		++pr->str; // skip '/' operator
		if (Parser_parse_and_expression(pr))
			return true;
	}

	return false;
}

bool Parser_parse_expression(Parser *pr) {
	return Parser_parse_or_expression_slash(pr);
}

bool parse_condition(const char* condition, args *data) {
	Parser pr;
	Parser_init(&pr, condition, data);
	return Parser_parse_expression(pr);
}

ADDT_EXPORT int addt(HSPEXINFO *hei) {
	args data = {
		hei,	// HSPEXINFO *hei
		(char *)hei->HspFunc_getproc(HSPVAR_FLAG_STR)->GetPtr(hei->HspFunc_prm_getpval()),	// char *buff 
		hei->HspFunc_prm_getpval(),		// PVal *gdataPVal
		hei->HspFunc_prm_getpval(),		// PVal *cdataPVal
		hei->HspFunc_prm_getpval(),		// PVal *sdataPVal
		hei->HspFunc_prm_getpval(),		// PVal *adataPVal
		hei->HspFunc_prm_getpval(),		// PVal *mdataPVal
		hei->HspFunc_prm_getpval(),		// PVal *cdatanPVal
		hei->HspFunc_prm_getpval(),		// PVal *invPVal
		hei->HspFunc_prm_getpval(),		// PVal *addtlvarPVal (応用以上)
		(char *)hei->HspFunc_getproc(HSPVAR_FLAG_STR)->GetPtr(hei->HspFunc_prm_getpval()),	// char *addtgvar (応用以上)
		*(int *)hei->HspFunc_getproc(HSPVAR_FLAG_INT)->GetPtr(hei->HspFunc_prm_getpval()),	// int unitid
		*(int *)hei->HspFunc_getproc(HSPVAR_FLAG_INT)->GetPtr(hei->HspFunc_prm_getpval()),	// int tcid
		*(int *)hei->HspFunc_getproc(HSPVAR_FLAG_INT)->GetPtr(hei->HspFunc_prm_getpval()),	// int ccid
		(char *)hei->HspFunc_getproc(HSPVAR_FLAG_STR)->GetPtr(hei->HspFunc_prm_getpval()),	// char *filter
	};
	if (*hei->er) return *hei->er;

	if (strstr(data.buff, "$") == NULL) return 0; //条件式がないならbuffを処理せず正常終了
	char *builder = (char *)malloc(strlen(data.buff) + 1);
	if (builder == NULL) return -1;
	builder[0] = '\0';

	bool added = false;
	int nestlim = -2;
	int nest = 0;
	int rankmax = 0;
	int rank = 0;

	char *lines[ADDT_LINE_NUMBER_MAX];
	int linenumber = split(data.buff, "\n", lines, ADDT_LINE_NUMBER_MAX); // \r(改行コードCR部)が残っているが必要なときの処理で十分

	for (int i = 0; i < linenumber; i++) {
		nest = removefs(lines[i]);
		if (lines[i][0] == '/' && lines[i][1] == '/') continue;
		if ((lines[i][0] == '$') || ((lines[i][0] == '%') && (lines[i][1] == '$'))) {
			/* 優先度 */
			if (strstr(lines[i], "[") != NULL) {
				char *rankc = strstr(lines[i], "[") + 6;
				*strstr(lines[i], "]") = '\0';
				rank = atoi(rankc);
				*strstr(lines[i], "[") = '\0';
			}
			else {
				rank = 0;
			}

			*lines[i]++;
			if (lines[i][0] == '$') {
				*lines[i]++;
			}
			/* 条件文の判定を行う */
			if (rankmax <= rank) {
				bool result = parse_condition(lines[i], &data);
				if (result) {
					nestlim = -2;
				}
				else {
					nestlim = nest - 1;
				}
			}
			else {
				nestlim = nest - 1;
			}
			/* 優先度が高いなら今まで一致した分を消す */
			if (nestlim == -2 && rankmax < rank) {
				rankmax = rank;
				strcpy(builder, "\r\n"); // \r(改行コードCR部)を足す必要がある
				added = false;
			}
		}
		else {
			if (nestlim == (-2) || nest <= nestlim) {
				if (added) strcat(builder, "\n");
				strcat(builder, lines[i]);
				added = true;
			}
		}
	}

	data.buff = strcpy(data.buff, builder);
	free(builder);
	return 0;
}

ADDT_EXPORT int txtmid(char *buff, char *section) {
	char *builder;

	if (strstr(buff, section) == NULL) {
		strcpy(buff, "");
		return 0;
	}
	else {
		builder = strstr(buff, section);
	}

	for (int i = 1; true; i++) {
		if (builder[i + 1] == '\0') break;
		if (builder[i - 1] == '\n' && builder[i] == '%' && builder[i + 1] != '$') {
			builder[i - 1] = '\0';
			break;
		}
	}

	strcpy(buff, builder);
	return 0;
}

ADDT_EXPORT int countchar(char *buff, char *target) {
	int count = 0;

	for (int i = 0; buff[i] != '\0'; i++) {
		if (buff[i] == target[0]) count++;
	}

	return count;
}