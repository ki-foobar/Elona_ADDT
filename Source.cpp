#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define ADDT_LINE_NUMBER_MAX 1024

#define ADDT_EXPORT extern "C" __declspec (dllexport)

#define gdata(i) intarray(pr->data->hei,pr->data->gdataPVal,i)
#define cdata(i,j) intmap(pr->data->hei,pr->data->cdataPVal,i,j)
#define adata(i,j) intmap(pr->data->hei,pr->data->adataPVal,i,j)
#define sdata(i,j) intmap(pr->data->hei,pr->data->sdataPVal,i,j)
#define mdata(i) intarray(pr->data->hei,pr->data->mdataPVal,i)
#define cdatan(i,j) strmap(pr->data->hei,pr->data->cdatanPVal,i,j)
#define rattach(dbl) rankflag ? rankattach(dbl) : (dbl)
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
	Parser_skip_whitespaces(pr);

	Token tok;
	if (!Parser_parse_one_token(pr, &tok))
		return false;

	if (tok.type == TOKEN_TYPE_INTEGER) {
		char *end_pos = pr->str;
		*parse_result = strtod(tok.str, &end_pos);
		pr->str = end_pos;
		return true;
	}

	if (tok.type != TOKEN_TYPE_IDENTIFIER)
		return false;

	int chara_id = pr->data->unitid;
	if (tok.len >= 3 && (tok.str[0] == 'P' && tok.str[1] == 'C')) {
		chara_id = 0;
		tok.str += 2;
		tok.len -= 2;
	}

	bool potentialflag = false, rankflag = false, expflag = false, revisionflag = false;

	while (1) {
		if (tok.str[tok.len - 1] == 'P') {
			potentialflag = true;
			tok.len -= 1;
		} else if (tok.str[tok.len - 1] == 'R') {
			rankflag = true;
			tok.len -= 1;
		} else if (tok.str[tok.len - 1] == 'E') {
			expflag = true;
			tok.len -= 1;
		} else {
			break;
		}
	}
	if (*pr->str == '+') {
		++pr->str;
		revisionflag = true;
		tok.len += 1;
	}

	switch (*tok.str) {
	case 'A':
		if (Token_equals_literal(&tok, "Anatomy")) {
			*parse_result = rattach(scomval(pr->data, 161, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Alchemy")) {
			*parse_result = rattach(scomval(pr->data, 178, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Axe")) {
			*parse_result = rattach(scomval(pr->data, 102, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Age")) {
			*parse_result = rattach((gdata(10) - cdata(21, chara_id)));
			return true;
		}
		break;
	case 'B':
		if (Token_equals_literal(&tok, "Blunt")) {
			*parse_result = rattach(scomval(pr->data, 103, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Bow")) {
			*parse_result = rattach(scomval(pr->data, 108, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'C':
		if (Token_equals_literal(&tok, "Constitution")) {
			*parse_result = rattach(scomval(pr->data, 11, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Charm")) {
			*parse_result = rattach(scomval(pr->data, 17, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Carpentry")) {
			*parse_result = rattach(scomval(pr->data, 176, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Cooking")) {
			*parse_result = rattach(scomval(pr->data, 184, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Casting")) {
			*parse_result = rattach(scomval(pr->data, 172, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Control_Magic")) {
			*parse_result = rattach(scomval(pr->data, 188, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Crossbow")) {
			*parse_result = rattach(scomval(pr->data, 109, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'D':
		if (Token_equals_literal(&tok, "Dexterity")) {
			*parse_result = rattach(scomval(pr->data, 12, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Dual_Wield")) {
			*parse_result = rattach(scomval(pr->data, 166, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Disarm_Trap")) {
			*parse_result = rattach(scomval(pr->data, 175, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Detection")) {
			*parse_result = rattach(scomval(pr->data, 159, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'E':
		if (Token_equals_literal(&tok, "Evasion")) {
			*parse_result = rattach(scomval(pr->data, 173, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Eye_of_Mind")) {
			*parse_result = rattach(scomval(pr->data, 186, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'F':
		if (Token_equals_literal(&tok, "Fishing")) {
			*parse_result = rattach(scomval(pr->data, 185, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Faith")) {
			*parse_result = rattach(scomval(pr->data, 181, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Firearm")) {
			*parse_result = rattach(scomval(pr->data, 110, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Fame")) {
			*parse_result = rattach(cdata(34, chara_id));
			return true;
		}
		break;
	case 'G':
		if (Token_equals_literal(&tok, "Greater_Evasion")) {
			*parse_result = rattach(scomval(pr->data, 187, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Gene_engineer")) {
			*parse_result = rattach(scomval(pr->data, 151, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Gardening")) {
			*parse_result = rattach(scomval(pr->data, 180, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'H':
		if (Token_equals_literal(&tok, "Healing")) {
			*parse_result = rattach(scomval(pr->data, 154, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Heavy_Armor")) {
			*parse_result = rattach(scomval(pr->data, 169, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Hp")) {
			*parse_result = rattach(cdata(50, chara_id));
			return true;
		}
		if (Token_equals_literal(&tok, "Height")) {
			*parse_result = rattach(cdata(19, chara_id));
			return true;
		}
		break;
	case 'I':
		if (Token_equals_literal(&tok, "Investing")) {
			*parse_result = rattach(scomval(pr->data, 160, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'J':
		if (Token_equals_literal(&tok, "Jeweler")) {
			*parse_result = rattach(scomval(pr->data, 179, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'L':
		if (Token_equals_literal(&tok, "Learning")) {
			*parse_result = rattach(scomval(pr->data, 14, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Lack")) {
			*parse_result = rattach(scomval(pr->data, 19, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Life")) {
			*parse_result = rattach(scomval(pr->data, 2, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Light_Armor")) {
			*parse_result = rattach(scomval(pr->data, 171, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Lock_Picking")) {
			*parse_result = rattach(scomval(pr->data, 158, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Literacy")) {
			*parse_result = rattach(scomval(pr->data, 150, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Long_Sword")) {
			*parse_result = rattach(scomval(pr->data, 100, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Level")) {
			*parse_result = rattach(cdata(38, chara_id));
			return true;
		}
		break;
	case 'M':
		switch (tok.str[1]) {
		case 'a':
			if (Token_equals_literal(&tok, "Magic")) {
				*parse_result = rattach(scomval(pr->data, 16, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Mana")) {
				*parse_result = rattach(scomval(pr->data, 3, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "MaxHp")) {
				*parse_result = rattach(cdata(51, chara_id));
				return true;
			}
			if (Token_equals_literal(&tok, "MaxSp")) {
				*parse_result = rattach(cdata(53, chara_id));
				return true;
			}
			if (Token_equals_literal(&tok, "MaxMp")) {
				*parse_result = rattach(cdata(56, chara_id));
				return true;
			}
			if (Token_equals_literal(&tok, "Marksman")) {
				*parse_result = rattach(scomval(pr->data, 189, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Magic_Capacity")) {
				*parse_result = rattach(scomval(pr->data, 164, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Magic_Device")) {
				*parse_result = rattach(scomval(pr->data, 174, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Martial_Arts")) {
				*parse_result = rattach(scomval(pr->data, 106, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			break;
		case 'e':
			if (Token_equals_literal(&tok, "Medium_Armor")) {
				*parse_result = rattach(scomval(pr->data, 170, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Memorization")) {
				*parse_result = rattach(scomval(pr->data, 165, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Meditation")) {
				*parse_result = rattach(scomval(pr->data, 155, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			break;
		case 'i':
			if (Token_equals_literal(&tok, "Mining")) {
				*parse_result = rattach(scomval(pr->data, 163, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			break;
		case 'p':
			if (Token_equals_literal(&tok, "Mp")) {
				*parse_result = rattach(cdata(55, chara_id));
				return true;
			}
			break;
		default:
			break;
		}
		break;
	case 'N':
		if (Token_equals_literal(&tok, "Negotiation")) {
			*parse_result = rattach(scomval(pr->data, 156, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'P':
		if (Token_equals_literal(&tok, "Perception")) {
			*parse_result = rattach(scomval(pr->data, 13, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Pickpocket")) {
			*parse_result = rattach(scomval(pr->data, 300, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Performer")) {
			*parse_result = rattach(scomval(pr->data, 183, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Polearm")) {
			*parse_result = rattach(scomval(pr->data, 104, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'R':
		if (Token_equals_literal(&tok, "Riding")) {
			*parse_result = rattach(scomval(pr->data, 301, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'S':
		switch (tok.str[1]) {
		case 'a':
			if (Token_equals_literal(&tok, "Sanity")) {
				*parse_result = rattach(cdata(34, chara_id));
				return true;
			}
			break;
		case 'c':
			if (Token_equals_literal(&tok, "Scythe")) {
				*parse_result = rattach(scomval(pr->data, 107, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			break;
		case 'e':
			if (Token_equals_literal(&tok, "Sense_Quality")) {
				*parse_result = rattach(scomval(pr->data, 162, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			break;
		case 'h':
			if (Token_equals_literal(&tok, "Shield")) {
				*parse_result = rattach(scomval(pr->data, 168, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Short_Sword")) {
				*parse_result = rattach(scomval(pr->data, 101, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			break;
		case 'p':
			if (Token_equals_literal(&tok, "Speed")) {
				*parse_result = rattach(scomval(pr->data, 18, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Sp")) {
				*parse_result = rattach(cdata(52, chara_id));
				return true;
			}
			break;
		case 't':
			if (Token_equals_literal(&tok, "Strength")) {
				*parse_result = rattach(scomval(pr->data, 10, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Stealth")) {
				*parse_result = rattach(scomval(pr->data, 157, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			if (Token_equals_literal(&tok, "Stave")) {
				*parse_result = rattach(scomval(pr->data, 105, chara_id, potentialflag, expflag, revisionflag));
				return true;
			}
			break;
		case 'u':
			if (Token_equals_literal(&tok, "Superb")) {
				*parse_result = rattach(5.0);
				return true;
			}
			break;
		default:
			break;
		}
		break;
	case 'T':
		if (Token_equals_literal(&tok, "Tactics")) {
			*parse_result = rattach(scomval(pr->data, 152, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Two_Hand")) {
			*parse_result = rattach(scomval(pr->data, 167, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Tailoring")) {
			*parse_result = rattach(scomval(pr->data, 177, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Traveling")) {
			*parse_result = rattach(scomval(pr->data, 182, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Throwing")) {
			*parse_result = rattach(scomval(pr->data, 111, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'W':
		if (Token_equals_literal(&tok, "Willpower")) {
			*parse_result = rattach(scomval(pr->data, 15, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Weight_Lifting")) {
			*parse_result = rattach(scomval(pr->data, 153, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "Weight")) {
			*parse_result = rattach(cdata(20, chara_id));
			return true;
		}
		break;
	case 'b':
		if (Token_equals_literal(&tok, "bad")) {
			*parse_result = rattach(2.0);
			return true;
		}
		break;
	case 'c':
		if (Token_equals_literal(&tok, "cold")) {
			*parse_result = rattach(scomval(pr->data, 51, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "chaos")) {
			*parse_result = rattach(scomval(pr->data, 59, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "cut")) {
			*parse_result = rattach(scomval(pr->data, 61, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_starts_with_literal(&tok, "cdata")) {
			*parse_result = rattach(sdata(atoi(tok.str + strlen("cdata")), pr->data->unitid));
			return true;
		}
		if (Token_starts_with_literal(&tok, "ccdata")) {
			*parse_result = rattach(sdata(atoi(tok.str + strlen("ccdata")), pr->data->ccid));
			return true;
		}
		if (Token_starts_with_literal(&tok, "csdata")) {
			*parse_result = rattach(sdata(atoi(tok.str + strlen("csdata")), pr->data->ccid));
			return true;
		}
		break;
	case 'd':
		if (Token_equals_literal(&tok, "darkness")) {
			*parse_result = rattach(scomval(pr->data, 53, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'f':
		if (Token_equals_literal(&tok, "fire")) {
			*parse_result = rattach(scomval(pr->data, 50, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'g':
		if (Token_starts_with_literal(&tok, "gdata")) {
			*parse_result = rattach(gdata(atoi(tok.str + strlen("gdata"))));
			return true;
		}
		if (Token_equals_literal(&tok, "great")) {
			*parse_result = rattach(4.0);
			return true;
		}
		if (Token_equals_literal(&tok, "good")) {
			*parse_result = rattach(3.0);
			return true;
		}
		break;
	case 'h':
		if (Token_equals_literal(&tok, "hopeless")) {
			*parse_result = rattach(1.0);
			return true;
		}
		break;
	case 'l':
		if (Token_equals_literal(&tok, "lightning")) {
			*parse_result = rattach(scomval(pr->data, 52, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'm':
		if (Token_equals_literal(&tok, "mind")) {
			*parse_result = rattach(scomval(pr->data, 54, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "magic")) {
			*parse_result = rattach(scomval(pr->data, 60, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'n':
		if (Token_equals_literal(&tok, "nether")) {
			*parse_result = rattach(scomval(pr->data, 56, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_equals_literal(&tok, "nerve")) {
			*parse_result = rattach(scomval(pr->data, 58, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 'p':
		if (Token_equals_literal(&tok, "poison")) {
			*parse_result = rattach(scomval(pr->data, 55, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		break;
	case 's':
		if (Token_equals_literal(&tok, "sound")) {
			*parse_result = rattach(scomval(pr->data, 57, chara_id, potentialflag, expflag, revisionflag));
			return true;
		}
		if (Token_starts_with_literal(&tok, "sdata")) {
			*parse_result = rattach(sdata(atoi(tok.str + strlen("sdata")), pr->data->unitid));
			return true;
		}
		break;
	case 't':
		if (Token_starts_with_literal(&tok, "tcdata")) {
			*parse_result = rattach(cdata(atoi(tok.str + strlen("tcdata"), pr->data->tcid));
			return true;
		}
		if (Token_starts_with_literal(&tok, "tsdata")) {
			*parse_result = rattach(sdata(atoi(tok.str + strlen("tsdata"), pr->data->tcid));
			return true;
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
			*parse_result = atof(value);
			return true;
		}
	} else {
		const char *value = getvalue(strarray(pr->data->hei, pr->data->addtlvarPVal, pr->data->unitid), &tok);
		if (*value) {
			*parse_result = atof(value);
			return true;
		}
	}

	*parse_result = 0.0;
	return true;
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
					return strcmp(cdatan(3, pr->data->unitid), cdatan(3, 0)) == 0;
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
					return strcmp(cdatan(2, pr->data->unitid), cdatan(2, 0)) == 0;
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
			return cbit(pr->data, pr->data->unitid, 969);
		}
		if (Token_equals_literal(&tok, "anorexia")) {
			return cbit(pr->data, pr->data->unitid, 986);
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
				return strcmp(cdatan(3, pr->data->unitid), cdatan(3, 0)) == 0;
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
			return cbit(pr->data, pr->data->unitid, 16);
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
			return cbit(pr->data, pr->data->unitid, 974);
		}
		break;
	case 'm':
		if (Token_equals_literal(&tok, "married")) {
			return cbit(pr->data, pr->data->unitid, 961);
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
				return strcmp(cdatan(2, pr->data->unitid), cdatan(2, 0)) == 0;
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
			return cbit(pr->data, pr->data->unitid, 975);
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
			return cbit(pr->data, pr->data->unitid, 966);
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
			return cbit(pr->data, pr->data->unitid, 968);
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