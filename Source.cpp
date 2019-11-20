#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define LINE_NUMBER_LIMIT 1024
#define KEY_NUMBER_LIMIT 512
#define CONDITION_NUMBER_LIMIT 64
#define WORD_NUMBER_LIMIT 8

#define EXPORT extern "C" __declspec (dllexport)

#define judge(con) return notflag ? !(con) : (con);
#define equals(s1,s2) (strcmp(s1,s2) == 0)
#define gdata(i) intarray(data.hei,data.gdataPVal,i)
#define cdata(i,j) intmap(data.hei,data.cdataPVal,i,j)
#define adata(i,j) intmap(data.hei,data.adataPVal,i,j)
#define sdata(i,j) intmap(data.hei,data.sdataPVal,i,j)
#define mdata(i) intarray(data.hei,data.mdataPVal,i)
#define cdatan(i,j) strmap(data.hei,data.cdatanPVal,i,j)
#define rattach(dbl) rankflag ? rankattach(dbl) : (dbl)

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

bool decimalonly(char *p){
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

void trim(char *s) {
	int i, j;

	for (i = strlen(s) - 1; i >= 0 && isspace(s[i]); i--);
	s[i + 1] = '\0';
	for (i = 0; isspace(s[i]); i++);
	if (i > 0) {
		j = 0;
		while (s[i]) s[j++] = s[i++];
		s[j] = '\0';
	}
}

int removefs(char *s) {
	int i = 0;
	while (s[i] != '\0' && s[i] == '	') i++;
	strcpy(s, &s[i]);

	return i;
}

int search(char *buff, char s){
	char *index = buff;
	while (index[0] != s && index[0] != '\0'){
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

const char* getvalue(char *vp, char *key){
	char keyc[259];
	strcpy(keyc, ";");
	strcat(keyc, key);
	strcat(keyc, ",");

	char *s = strstr(vp, keyc);
	int len = strlen(keyc);

	if (s != NULL){
		int vlen = search(s + len, ';');
		char *value = (char*)malloc(vlen + 1);
		strncpy(value, s + len, vlen);
		value[vlen] = '\0';
		return value;
	}
	else {
		return "-1";
	}
}

double rankattach(double value){
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

int intarray(HSPEXINFO *hei, PVal *intarray, int index){
	HspVarCoreReset(intarray);
	hei->HspFunc_array(intarray, index);
	return *(int *)hei->HspFunc_getproc(HSPVAR_FLAG_INT)->GetPtr(intarray);
}

int intmap(HSPEXINFO *hei, PVal *intmap, int index1, int index2){
	HspVarCoreReset(intmap);
	hei->HspFunc_array(intmap, index1);
	hei->HspFunc_array(intmap, index2);
	return *(int *)hei->HspFunc_getproc(HSPVAR_FLAG_INT)->GetPtr(intmap);
}

char* strarray(HSPEXINFO *hei, PVal *strarray, int index){
	HspVarCoreReset(strarray);
	hei->HspFunc_array(strarray, index);
	return (char *)hei->HspFunc_getproc(HSPVAR_FLAG_STR)->GetPtr(strarray);
}

char* strmap(HSPEXINFO *hei, PVal *strmap, int index1, int index2){
	HspVarCoreReset(strmap);
	hei->HspFunc_array(strmap, index1);
	hei->HspFunc_array(strmap, index2);
	return (char *)hei->HspFunc_getproc(HSPVAR_FLAG_STR)->GetPtr(strmap);
}

bool cbit(args data, int unitid, int bitid){
	int checker = 1;
	checker <<= bitid % 32;
	return ((checker & cdata(450 + (bitid / 32), unitid)) == 0) ? false : true;
}

double scomval(args data, int index, int unitid, bool potentialflag, bool expflag, bool revisionflag){
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

double comparisonval(args data, char *target){
	bool potentialflag = false, rankflag = false, expflag = false, revisionflag = false, filterenable = (!equals(data.filter, "/none/"));
	int sdataindex = 0, referid = data.unitid;

	if (target[0] == 'P' && target[1] == 'C') {
		referid = 0;
		target += 2;
	}
	for (int i = strlen(target) - 1; i != 0; i--){
		if (target[i] == 'P'){
			potentialflag = true;
		}
		else if (target[i] == 'R'){
			rankflag = true;
		}
		else if (target[i] == 'E'){
			expflag = true;
		}
		else if (target[i] == '+'){
			revisionflag = true;
		}
		else {
			target[i + 1] = '\0';
			break;
		}
	}

	if (decimalonly(target)) {
		if (filterenable && strstr(data.filter, "/comparisonval-decimal/") != NULL) return -1.0;
		return rattach(atof(target));
	}

	switch (target[0]) {
	case 'A':
		if (equals(target, "Anatomy")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 161, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Alchemy")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 178, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Axe")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 102, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Age")) {
			if (filterenable && strstr(data.filter, "/comparisonval-detail/") != NULL) return -1.0;
			return rattach((double)(gdata(10) - cdata(21, referid)));
		}
		break;
	case 'B':
		if (equals(target, "Blunt")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 103, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Bow")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 108, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'C':
		if (equals(target, "Constitution")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach(scomval(data, 11, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Charm")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach(scomval(data, 17, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Carpentry")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 176, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Cooking")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 184, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Casting")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 172, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Control_Magic")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 188, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Crossbow")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 109, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'D':
		if (equals(target, "Dexterity")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach(scomval(data, 12, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Dual_Wield")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 166, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Disarm_Trap")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 175, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Detection")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 159, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'E':
		if (equals(target, "Evasion")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 173, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Eye_of_Mind")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 186, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'F':
		if (equals(target, "Fishing")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 185, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Faith")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 181, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Firearm")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 110, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Fame")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach((double)cdata(34, referid));
		}
		break;
	case 'G':
		if (equals(target, "Greater_Evasion")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 187, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Gene_engineer")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 151, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Gardening")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 180, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'H':
		if (equals(target, "Healing")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 154, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Heavy_Armor")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 169, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Hp")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach((double)cdata(50, referid));
		}
		if (equals(target, "Height")) {
			if (filterenable && strstr(data.filter, "/comparisonval-detail/") != NULL) return -1.0;
			return rattach((double)cdata(19, referid));
		}
		break;
	case 'I':
		if (equals(target, "Investing")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 160, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'J':
		if (equals(target, "Jeweler")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 179, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'L':
		if (equals(target, "Learning")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach(scomval(data, 14, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Lack")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach(scomval(data, 19, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Life")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach(scomval(data, 2, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Light_Armor")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 171, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Lock_Picking")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 158, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Literacy")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 150, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Long_Sword")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 100, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Level")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach((double)cdata(38, referid));
		}
		break;
	case 'M':
		switch (target[1]) {
		case 'a':
			if (equals(target, "Magic")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach(scomval(data, 16, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Mana")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach(scomval(data, 3, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "MaxHp")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach((double)cdata(51, referid));
			}
			if (equals(target, "MaxSp")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach((double)cdata(53, referid));
			}
			if (equals(target, "MaxMp")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach((double)cdata(56, referid));
			}
			if (equals(target, "Marksman")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 189, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Magic_Capacity")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 164, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Magic_Device")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 174, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Martial_Arts")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 106, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'e':
			if (equals(target, "Medium_Armor")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 170, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Memorization")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 165, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Meditation")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 155, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'i':
			if (equals(target, "Mining")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 163, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'p':
			if (equals(target, "Mp")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach((double)cdata(55, referid));
			}
			break;
		default:
			break;
		}
		break;
	case 'N':
		if (equals(target, "Negotiation")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 156, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'P':
		if (equals(target, "Perception")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach(scomval(data, 13, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Pickpocket")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 300, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Performer")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 183, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Polearm")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 104, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'R':
		if (equals(target, "Riding")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 301, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'S':
		switch (target[1]){
		case 'a':
			if (equals(target, "Sanity")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach((double)cdata(34, referid));
			}
			break;
		case 'c':
			if (equals(target, "Scythe")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 107, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'e':
			if (equals(target, "Sense_Quality")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 162, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'h':
			if (equals(target, "Shield")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 168, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Short_Sword")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 101, referid, potentialflag, expflag, revisionflag));
			}
			break;
		case 'p':
			if (equals(target, "Speed")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach(scomval(data, 18, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Sp")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach((double)cdata(52, referid));
			}
			break;
		case 't':
			if (equals(target, "Strength")) {
				if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
				return rattach(scomval(data, 10, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Stealth")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
				return rattach(scomval(data, 157, referid, potentialflag, expflag, revisionflag));
			}
			if (equals(target, "Stave")) {
				if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
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
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 152, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Two_Hand")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 167, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Tailoring")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 177, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Traveling")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 182, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Throwing")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 111, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'W':
		if (equals(target, "Willpower")) {
			if (filterenable && strstr(data.filter, "/comparisonval-ability/") != NULL) return -1.0;
			return rattach(scomval(data, 15, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Weight_Lifting")) {
			if (filterenable && strstr(data.filter, "/comparisonval-skill/") != NULL) return -1.0;
			return rattach(scomval(data, 153, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "Weight")) {
			if (filterenable && strstr(data.filter, "/comparisonval-detail/") != NULL) return -1.0;
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
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 51, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "chaos")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 59, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "cut")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 61, referid, potentialflag, expflag, revisionflag));
		}
		if (strstr(target, "cdata") == target) {
			if (filterenable && strstr(data.filter, "/comparisonval-data/") != NULL) return -1.0;
			target += 5;
			return rattach((double)cdata(atoi(target), data.unitid));
		}
		if (strstr(target, "ccdata") == target) {
			if (filterenable && strstr(data.filter, "/comparisonval-data/") != NULL) return -1.0;
			target += 6;
			return rattach((double)cdata(atoi(target), data.ccid));
		}
		if (strstr(target, "csdata") == target) {
			if (filterenable && strstr(data.filter, "/comparisonval-data/") != NULL) return -1.0;
			target += 6;
			return rattach((double)sdata(atoi(target), data.ccid));
		}
		break;
	case 'd':
		if (equals(target, "darkness")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 53, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'f':
		if (equals(target, "fire")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 50, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'g':
		if (strstr(target, "gdata") == target) {
			if (filterenable && strstr(data.filter, "/comparisonval-data/") != NULL) return -1.0;
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
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 52, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'm':
		if (equals(target, "mind")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 54, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "magic")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 60, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'n':
		if (equals(target, "nether")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 56, referid, potentialflag, expflag, revisionflag));
		}
		if (equals(target, "nerve")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 58, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 'p':
		if (equals(target, "poison")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 55, referid, potentialflag, expflag, revisionflag));
		}
		break;
	case 's':
		if (equals(target, "sound")) {
			if (filterenable && strstr(data.filter, "/comparisonval-resistance/") != NULL) return -1.0;
			return rattach(scomval(data, 57, referid, potentialflag, expflag, revisionflag));
		}
		if (strstr(target, "sdata") == target) {
			if (filterenable && strstr(data.filter, "/comparisonval-data/") != NULL) return -1.0;
			target += 5;
			return rattach((double)sdata(atoi(target), data.unitid));
		}
		break;
	case 't':
		if (strstr(target, "tcdata") == target) {
			if (filterenable && strstr(data.filter, "/comparisonval-data/") != NULL) return -1.0;
			target += 6;
			return rattach((double)cdata(atoi(target), data.tcid));
		}
		if (strstr(target, "tsdata") == target) {
			if (filterenable && strstr(data.filter, "/comparisonval-data/") != NULL) return -1.0;
			target += 6;
			return rattach((double)sdata(atoi(target), data.tcid));
		}
		break;
	default:
		break;
	}

	if (filterenable && strstr(data.filter, "/comparisonval-key/") != NULL) return -1.0;
	if (strstr(target, "global:") == target){
		target += 7;
		return atof(getvalue(data.addtgvar, target));
	}
	return atof(getvalue(strarray(data.hei, data.addtlvarPVal, referid), target));
}

const char* strcomparisonstr(args data, char *target){
	if (strstr(target, "\"") == target) {
		return chartrim(target, '"');
	}

	int referid = data.unitid;
	if (target[0] == 'P'&&target[1] == 'C'){
		target += 2;
		referid = 0;
	}

	switch (target[0]){
	case 'A':
		if (equals(target, "Aka")) {
			return cdatan(1, referid);
		}
		if (equals(target, "Age")) {
			static char s[16];
			sprintf(s, "%d", gdata(10) - cdata(21, referid));
			return s;
		}
		break;
	case 'C':
		if (equals(target, "Class")) {
			return cdatan(3, referid);
		}
		if (equals(target, "Cash")) {
			static char s[16];
			sprintf(s, "%d", cdata(30, referid));
			return s;
		}
		break;
	case 'D':
		if (equals(target, "Date")) {
			static char s[16];
			sprintf(s, "%d", gdata(12));
			return s;
		}
		break;
	case 'F':
		if (equals(target, "Fame")) {
			static char s[16];
			sprintf(s, "%d", cdata(34, referid));
			return s;
		}
		break;
	case 'H':
		if (equals(target, "Height")) {
			static char s[16];
			sprintf(s, "%d", cdata(19, referid));
			return s;
		}
		if (equals(target, "Hour")) {
			static char s[16];
			sprintf(s, "%d", gdata(13));
			return s;
		}
		break;
	case 'I':
		if (equals(target, "Impression")) {
			static char s[16];
			sprintf(s, "%d", cdata(17, referid));
			return s;
		}
		break;
	case 'K':
		if (equals(target, "Karma")) {
			static char s[16];
			sprintf(s, "%d", cdata(49, 0));
			return s;
		}
		break;
	case 'M':
		if (equals(target, "Month")) {
			static char s[16];
			sprintf(s, "%d", gdata(11));
			return s;
		}
		break;
	case 'N':
		if (equals(target, "Name")) {
			return cdatan(0, referid);
		}
		break;
	case 'R':
		if (equals(target, "Race")) {
			return cdatan(2, referid);
		}
		if (equals(target, "Religion")) {
			switch (cdata(61, referid)){
			case 0:
				return "無のエイス";
			case 1:
				return "機械のマニ";
			case 2:
				return "風のルルウィ";
			case 3:
				return "元素のイツパロトル";
			case 4:
				return "幸運のエヘカトル";
			case 5:
				return "地のオパートス";
			case 6:
				return "癒しのジュア";
			case 7:
				return "収穫のクミロミ";
			default:
				return "-1";
			}
		}
		break;
	case 'W':
		if (equals(target, "Weight")) {
			static char s[16];
			sprintf(s, "%d", cdata(20, referid));
			return s;
		}
		break;
	case 'Y':
		if (equals(target, "Year")) {
			static char s[16];
			sprintf(s, "%d", gdata(10));
			return s;
		}
		break;
	case 'n':
		if (equals(target, "nReligion")) {
			switch (cdata(61, referid)){
			case 0:
				return "エイス";
			case 1:
				return "マニ";
			case 2:
				return "ルルウィ";
			case 3:
				return "イツパロトル";
			case 4:
				return "エヘカトル";
			case 5:
				return "オパートス";
			case 6:
				return "ジュア";
			case 7:
				return "クミロミ";
			default:
				return "-1";
			}
		}
		break;
	default:
		break;
	}

	if (!equals(getvalue(strarray(data.hei, data.addtlvarPVal, data.unitid), target), "")) {
		return getvalue(strarray(data.hei, data.addtlvarPVal, data.unitid), target);
	}
	if (7 < strlen(target)) {
		target += 7;
		if (!equals(getvalue(data.addtgvar, target), "")) {
			return getvalue(data.addtgvar, target);
		}
	}
	return "-1";
}

bool judgement(char condition[], args data){
	if (condition[strlen(condition) - 1] == '\r') {
		condition[strlen(condition) - 1] = '\0';
	}
	trim(condition);

	bool notflag = ((condition[0] == '!') ? true : false);
	bool filterenable = (!equals(data.filter, "/none/"));
	if (notflag) *condition++;
	char *words[WORD_NUMBER_LIMIT];
	int wordnumber = split(condition, " ", words, WORD_NUMBER_LIMIT);

	// judgeにはreturnが含まれているので凡その場合においてelseの記述は無意味

	switch (condition[0]){
	case 'P':
		switch (condition[2]){
		case 'a':
			if (equals(words[0], "PCaction")){
				if (filterenable && strstr(data.filter, "/PCaction/") != NULL) return false;
				if (equals(words[1], "Performance")){
					judge(cdata(140, 0) == 6);
				}
				if (equals(words[1], "Dig")){
					judge(cdata(140, 0) == 5);
				}
				if (equals(words[1], "Reading")){
					judge(cdata(140, 0) == 2);
				}
				if (equals(words[1], "Fishing")){
					judge(cdata(140, 0) == 7);
				}
				if (equals(words[1], "Harvesting")){
					judge(cdata(140, 0) == 8);
				}
				if (equals(words[1], "Search")){
					judge(cdata(140, 0) == 9);
				}
				if (equals(words[1], "Eat")){
					judge(cdata(140, 0) == 1);
				}
				return false;
			}
			break;
		case 'c':
			if (equals(words[0], "PCcondition")){
				if (filterenable && strstr(data.filter, "/PCcondition/") != NULL) return false;
				if (equals(words[1], "Normal")){
					judge(cdata(250, 0) == 0 && cdata(251, 0) == 0 && cdata(252, 0) == 0 && cdata(253, 0) == 0 && cdata(254, 0) == 0 && cdata(255, 0) == 0 && cdata(256, 0) == 0 && cdata(257, 0) == 0 && cdata(258, 0) == 0 && cdata(259, 0) == 0 && cdata(260, 0) == 0 && cdata(261, 0) == 0 && cdata(264, 0) == 0);
				}
				if (equals(words[1], "Poisoned")){
					judge(0 < cdata(250, 0));
				}
				if (equals(words[1], "Sleep")){
					judge(0 < cdata(251, 0));
				}
				if (equals(words[1], "Paralyzed")){
					judge(0 < cdata(252, 0));
				}
				if (equals(words[1], "Blinded")){
					judge(0 < cdata(253, 0));
				}
				if (equals(words[1], "Confused")){
					judge(0 < cdata(254, 0));
				}
				if (equals(words[1], "Fear")){
					judge(0 < cdata(255, 0));
				}
				if (equals(words[1], "Dim")){
					judge(0 < cdata(256, 0));
				}
				if (equals(words[1], "Drunk")){
					judge(0 < cdata(257, 0));
				}
				if (equals(words[1], "Bleeding")){
					judge(0 < cdata(258, 0));
				}
				if (equals(words[1], "Wet")){
					judge(0 < cdata(259, 0));
				}
				if (equals(words[1], "Insane")){
					judge(0 < cdata(260, 0));
				}
				if (equals(words[1], "Sick")){
					judge(0 < cdata(261, 0));
				}
				if (equals(words[1], "Fury")){
					judge(0 < cdata(264, 0));
				}
				if (equals(words[1], "sleepiness")){
					if (filterenable && strstr(data.filter, "/PCcondition-sleepiness/") != NULL) return false;
					if (wordnumber == 3){
						judge(gdata(90) == atoi(words[2]));
					}
					if (wordnumber == 4){
						if (equals(words[2], "-")){
							judge(gdata(90) <= atoi(words[3]));
						}
						else {
							judge(atoi(words[2]) <= gdata(90));
						}
					}
					if (wordnumber == 5){
						judge(atoi(words[2]) <= gdata(90) && gdata(90) <= atoi(words[4]));
					}
					return false;
				}
			}
			if (equals(words[0], "PCcash")){
				if (filterenable && strstr(data.filter, "/PCcash/") != NULL) return false;
				if (wordnumber == 2){
					judge(cdata(30, 0) == atoi(words[1]));
				}
				if (wordnumber == 3){
					if (equals(words[1], "-")){
						judge(cdata(30, 0) <= atoi(words[2]));
					}
					else {
						judge(atoi(words[1]) <= cdata(30, 0));
					}
				}
				if (wordnumber == 4){
					judge(atoi(words[1]) <= cdata(30, 0) && cdata(30, 0) <= atoi(words[3]));
				}
				return false;
			}
			if (equals(words[0], "PCclass")){
				if (filterenable && strstr(data.filter, "/PCclass/") != NULL) return false;
				if (equals(words[1], "same")){
					judge(equals(cdatan(3, data.unitid), cdatan(3, 0)));
				}
				else {
					judge(equals(cdatan(3, 0), words[1]));
				}
			}
			break;
		case 'f':
			if (equals(words[0], "PCfame")){
				if (filterenable && strstr(data.filter, "/PCfame/") != NULL) return false;
				if (wordnumber == 2){
					judge(cdata(34, 0) == atoi(words[1]));
				}
				if (wordnumber == 3){
					if (equals(words[1], "-")){
						judge(cdata(34, 0) <= atoi(words[2]));
					}
					else {
						judge(atoi(words[1]) <= cdata(34, 0));
					}
				}
				if (wordnumber == 4){
					judge(atoi(words[1]) <= cdata(34, 0) && cdata(34, 0) <= atoi(words[3]));
				}
				return false;
			}
			break;
		case 'r':
			if (equals(words[0], "PCreligion")){
				if (filterenable && strstr(data.filter, "/PCreligion/") != NULL) return false;
				if (equals(words[1], "same")){
					judge(cdata(61, data.unitid) == cdata(61, 0));
				}
				if (equals(words[1], "Eyth")){
					judge(cdata(61, 0) == 0);
				}
				if (equals(words[1], "Mani")){
					judge(cdata(61, 0) == 1);
				}
				if (equals(words[1], "Lulwy")){
					judge(cdata(61, 0) == 2);
				}
				if (equals(words[1], "Itzpalt")){
					judge(cdata(61, 0) == 3);
				}
				if (equals(words[1], "Ehekatl")){
					judge(cdata(61, 0) == 4);
				}
				if (equals(words[1], "Opatos")){
					judge(cdata(61, 0) == 5);
				}
				if (equals(words[1], "Jure")){
					judge(cdata(61, 0) == 6);
				}
				if (equals(words[1], "Kumiromi")){
					judge(cdata(61, 0) == 7);
				}
				return false;
			}
			if (equals(words[0], "PCrace")){
				if (filterenable && strstr(data.filter, "/PCrace/") != NULL) return false;
				if (equals(words[1], "same")){
					judge(equals(cdatan(2, data.unitid), cdatan(2, 0)));
				}
				else {
					judge(equals(cdatan(2, 0), words[1]));
				}
			}
			break;
		case 's':
			if (equals(words[0], "PCsex")){
				if (filterenable && strstr(data.filter, "/PCsex/") != NULL) return false;
				if (equals(words[1], "same")){
					judge(cdata(8, data.unitid) == cdata(8, 0));
				}
				if (equals(words[1], "Male")){
					judge(cdata(8, 0) == 0);
				}
				if (equals(words[1], "Female")){
					judge(cdata(8, 0) == 1);
				}
				return false;
			}
			break;
		default:
			break;
		}
		break;
	case 'a':
		if (equals(words[0], "action")){
			if (filterenable && strstr(data.filter, "/action/") != NULL) return false;
			if (equals(words[1], "Performance")){
				judge(cdata(140, data.unitid) == 6);
			}
			if (equals(words[1], "Eat")){
				judge(cdata(140, data.unitid) == 1);
			}
			return false;
		}
		if (equals(words[0], "agreement")){
			if (filterenable && strstr(data.filter, "/agreement/") != NULL) return false;
			judge(cbit(data, data.unitid, 969));
		}
		if (equals(words[0], "anorexia")){
			if (filterenable && strstr(data.filter, "/anorexia/") != NULL) return false;
			judge(cbit(data, data.unitid, 986));
		}
		break;
	case 'c':
		if (equals(words[0], "condition")){
			if (filterenable && strstr(data.filter, "/condition/") != NULL) return false;
			if (equals(words[1], "Normal")){
				judge(cdata(250, data.unitid) == 0 && cdata(251, data.unitid) == 0 && cdata(252, data.unitid) == 0 && cdata(253, data.unitid) == 0 && cdata(254, data.unitid) == 0 && cdata(255, data.unitid) == 0 && cdata(256, data.unitid) == 0 && cdata(257, data.unitid) == 0 && cdata(258, data.unitid) == 0 && cdata(259, data.unitid) == 0 && cdata(260, data.unitid) == 0 && cdata(261, data.unitid) == 0 && cdata(264, data.unitid) == 0);
			}
			if (equals(words[1], "Poisoned")){
				judge(0 < cdata(250, data.unitid));
			}
			if (equals(words[1], "Sleep")){
				judge(0 < cdata(251, data.unitid));
			}
			if (equals(words[1], "Paralyzed")){
				judge(0 < cdata(252, data.unitid));
			}
			if (equals(words[1], "Blinded")){
				judge(0 < cdata(253, data.unitid));
			}
			if (equals(words[1], "Confused")){
				judge(0 < cdata(254, data.unitid));
			}
			if (equals(words[1], "Fear")){
				judge(0 < cdata(255, data.unitid));
			}
			if (equals(words[1], "Dim")){
				judge(0 < cdata(256, data.unitid));
			}
			if (equals(words[1], "Drunk")){
				judge(0 < cdata(257, data.unitid));
			}
			if (equals(words[1], "Bleeding")){
				judge(0 < cdata(258, data.unitid));
			}
			if (equals(words[1], "Wet")){
				judge(0 < cdata(259, data.unitid));
			}
			if (equals(words[1], "Insane")){
				judge(0 < cdata(260, data.unitid));
			}
			if (equals(words[1], "Sick")){
				judge(0 < cdata(261, data.unitid));
			}
			if (equals(words[1], "Fury")){
				judge(0 < cdata(264, data.unitid));
			}
		}
		if (equals(words[0], "cash")){
			if (filterenable && strstr(data.filter, "/cash/") != NULL) return false;
			if (wordnumber == 2){
				judge(cdata(30, data.unitid) == atoi(words[1]));
			}
			if (wordnumber == 3){
				if (equals(words[1], "-")){
					judge(cdata(30, data.unitid) <= atoi(words[2]));
				}
				else {
					judge(atoi(words[1]) <= cdata(30, data.unitid));
				}
			}
			if (wordnumber == 4){
				judge(atoi(words[1]) <= cdata(30, data.unitid) && cdata(30, data.unitid) <= atoi(words[3]));
			}
			return false;
		}
		if (equals(words[0], "class")){
			if (filterenable && strstr(data.filter, "/class/") != NULL) return false;
			if (equals(words[1], "same")){
				judge(equals(cdatan(3, data.unitid), cdatan(3, 0)));
			}
			else {
				judge(equals(cdatan(3, data.unitid), words[1]));
			}
		}
		if (equals(words[0], "comparison")) {
			if (filterenable && strstr(data.filter, "/comparison/") != NULL) return false;
			if (equals(words[2], "==")) {
				judge(comparisonval(data, words[1]) == comparisonval(data, words[3]));
			}
			if (equals(words[2], "<=") || equals(words[2], "=<")) {
				judge(comparisonval(data, words[1]) <= comparisonval(data, words[3]));
			}
			if (equals(words[2], "<")) {
				judge(comparisonval(data, words[1]) < comparisonval(data, words[3]));
			}
			if (equals(words[2], ">=") || equals(words[2], "=>")) {
				judge(comparisonval(data, words[1]) >= comparisonval(data, words[3]));
			}
			if (equals(words[2], ">")) {
				judge(comparisonval(data, words[1]) > comparisonval(data, words[3]));
			}
			if (strstr(words[2], "<*") == words[2]) {
				words[2] += 2;
				judge(comparisonval(data, words[1]) * atof(words[2]) <= comparisonval(data, words[3]));
			}
			if (strstr(words[2], ">*") == words[2]) {
				words[2] += 2;
				judge(comparisonval(data, words[1]) >= comparisonval(data, words[3]) * atof(words[2]));
			}
			if (strstr(words[2], "<+") == words[2]) {
				words[2] += 2;
				judge(comparisonval(data, words[1]) + atof(words[2]) <= comparisonval(data, words[3]));
			}
			if (strstr(words[2], ">+") == words[2]) {
				words[2] += 2;
				judge(comparisonval(data, words[1]) >= comparisonval(data, words[3]) + atof(words[2]));
			}
			return false;
		}
		break;
	case 'f':
		if (equals(words[0], "fame")){
			if (filterenable && strstr(data.filter, "/fame/") != NULL) return false;
			if (wordnumber == 2){
				judge(cdata(34, data.unitid) == atoi(words[1]));
			}
			if (wordnumber == 3){
				if (equals(words[1], "-")){
					judge(cdata(34, data.unitid) <= atoi(words[2]));
				}
				else {
					judge(atoi(words[1]) <= cdata(34, data.unitid));
				}
			}
			if (wordnumber == 4){
				judge(atoi(words[1]) <= cdata(34, data.unitid) && cdata(34, data.unitid) <= atoi(words[3]));
			}
			return false;
		}
		if (equals(words[0], "false")){
			if (filterenable && strstr(data.filter, "/false/") != NULL) return false;
			judge(false);
		}
		break;
	case 'i':
		if (equals(words[0], "impression")){
			if (filterenable && strstr(data.filter, "/impression/") != NULL) return false;
			if (equals(words[1], "Foe")){
				judge(cdata(17, data.unitid) <= 9);
			}
			if (equals(words[1], "Hate")){
				judge(10 <= cdata(17, data.unitid) && cdata(17, data.unitid) <= 24);
			}
			if (equals(words[1], "Annoying")){
				judge(25 <= cdata(17, data.unitid) && cdata(17, data.unitid) <= 39);
			}
			if (equals(words[1], "Normal")){
				judge(40 <= cdata(17, data.unitid) && cdata(17, data.unitid) <= 74);
			}
			if (equals(words[1], "Amiable")){
				judge(75 <= cdata(17, data.unitid) && cdata(17, data.unitid) <= 99);
			}
			if (equals(words[1], "Friend")){
				judge(100 <= cdata(17, data.unitid) && cdata(17, data.unitid) <= 149);
			}
			if (equals(words[1], "Fellow")){
				judge(150 <= cdata(17, data.unitid) && cdata(17, data.unitid) <= 199);
			}
			if (equals(words[1], "Soul_Mate")){
				judge(200 <= cdata(17, data.unitid) && cdata(17, data.unitid) <= 299);
			}
			if (equals(words[1], "Love")){
				judge(cdata(17, data.unitid) >= 300);
			}
			if (filterenable && strstr(data.filter, "/impression-value/") != NULL) return false;
			if (wordnumber == 2){
				judge(cdata(17, data.unitid) == atoi(words[1]));
			}
			if (wordnumber == 3){
				if (equals(words[1], "-")){
					judge(cdata(17, data.unitid) <= atoi(words[2]));
				}
				else {
					judge(atoi(words[1]) <= cdata(17, data.unitid));
				}
			}
			if (wordnumber == 4){
				judge(atoi(words[1]) <= cdata(17, data.unitid) && cdata(17, data.unitid) <= atoi(words[3]));
			}
			return false;
		}
		if (equals(words[0], "incognito")){
			if (filterenable && strstr(data.filter, "/incognito/") != NULL) return false;
			judge(cbit(data, data.unitid, 16));
		}
		break;
	case 'k':
		if (equals(words[0], "karma")){
			if (filterenable && strstr(data.filter, "/karma/") != NULL) return false;
			if (wordnumber == 2){
				judge(cdata(49, 0) == atoi(words[1]));
			}
			if (wordnumber == 3){
				if (equals(words[1], "-")){
					judge(cdata(49, 0) <= atoi(words[2]));
				}
				else {
					judge(atoi(words[1]) <= gdata(90));
				}
			}
			if (wordnumber == 4){
				judge(atoi(words[1]) <= cdata(49, 0) && cdata(49, 0) <= atoi(words[3]));
			}
			return false;
		}
		break;
	case 'l':
		if (equals(words[0], "layhand")){
			if (filterenable && strstr(data.filter, "/layhand/") != NULL) return false;
			judge(cbit(data, data.unitid, 974));
		}
		break;
	case 'm':
		if (equals(words[0], "married")){
			if (filterenable && strstr(data.filter, "/married/") != NULL) return false;
			judge(cbit(data, data.unitid, 961));
		}
		break;
	case 'p':
		if (equals(words[0], "pet")){
			if (filterenable && strstr(data.filter, "/pet/") != NULL) return false;
			judge(cdata(9, data.unitid) == 10);
		}
		break;
	case 'r':
		if (equals(words[0], "religion")){
			if (filterenable && strstr(data.filter, "/religion/") != NULL) return false;
			if (equals(words[1], "same")){
				judge(cdata(61, data.unitid) == cdata(61, 0));
			}
			if (equals(words[1], "Eyth")){
				judge(cdata(61, data.unitid) == 0);
			}
			if (equals(words[1], "Mani")){
				judge(cdata(61, data.unitid) == 1);
			}
			if (equals(words[1], "Lulwy")){
				judge(cdata(61, data.unitid) == 2);
			}
			if (equals(words[1], "Itzpalt")){
				judge(cdata(61, data.unitid) == 3);
			}
			if (equals(words[1], "Ehekatl")){
				judge(cdata(61, data.unitid) == 4);
			}
			if (equals(words[1], "Opatos")){
				judge(cdata(61, data.unitid) == 5);
			}
			if (equals(words[1], "Jure")){
				judge(cdata(61, data.unitid) == 6);
			}
			if (equals(words[1], "Kumiromi")){
				judge(cdata(61, data.unitid) == 7);
			}
			return false;
		}
		if (equals(words[0], "race")){
			if (filterenable && strstr(data.filter, "/race/") != NULL) return false;
			if (equals(words[1], "same")){
				judge(equals(cdatan(2, data.unitid), cdatan(2, 0)));
			}
			else {
				judge(equals(cdatan(2, data.unitid), words[1]));
			}
		}
		if (equals(words[0], "random")){
			if (filterenable && strstr(data.filter, "/random/") != NULL) return false;
			srand(GetTickCount());
			judge(rand() % 100 < atoi(words[1]));
		}
		if (equals(words[0], "ridden")){
			if (filterenable && strstr(data.filter, "/ridden/") != NULL) return false;
			judge(cbit(data, data.unitid, 975));
		}
		break;
	case 's':
		if (equals(words[0], "sex")){
			if (filterenable && strstr(data.filter, "/sex/") != NULL) return false;
			if (equals(words[1], "same")){
				judge(cdata(8, data.unitid) == cdata(8, 0));
			}
			if (equals(words[1], "Male")){
				judge(cdata(8, data.unitid) == 0);
			}
			if (equals(words[1], "Female")){
				judge(cdata(8, data.unitid) == 1);
			}
			return false;
		}
		if (equals(words[0], "stethoscope")){
			if (filterenable && strstr(data.filter, "/stethoscope/") != NULL) return false;
			judge(cbit(data, data.unitid, 966));
		}
		if (equals(words[0], "strcomparison")) {
			if (filterenable && strstr(data.filter, "/strcomparison/") != NULL) return false;
			if (equals(words[2], "equal")) {
				judge(equals(strcomparisonstr(data, words[1]), strcomparisonstr(data, words[3])));
			}
			if (equals(words[2], "instr")) {
				judge(strstr(strcomparisonstr(data, words[1]), strcomparisonstr(data, words[3])) != NULL);
			}
			return false;
		}
		break;
	case 't':
		if (equals(words[0], "tied")){
			if (filterenable && strstr(data.filter, "/tied/") != NULL) return false;
			judge(cbit(data, data.unitid, 968));
		}
		if (equals(words[0], "true")){
			if (filterenable && strstr(data.filter, "/true/") != NULL) return false;
			judge(true);
		}
		break;
	case 'w':
		if (equals(words[0], "when")){
			if (filterenable && strstr(data.filter, "/when/") != NULL) return false;
			if (equals(words[1], "year")){
				if (wordnumber == 3) {
					judge(gdata(10) == atoi(words[2]));
				}
				if (wordnumber == 4) {
					if (equals(words[2], "-")){
						judge(gdata(10) <= atoi(words[3]));
					}
					else {
						judge(gdata(10) >= atoi(words[2]));
					}
				}
				if (wordnumber == 5) {
					judge(atoi(words[2]) <= gdata(10) && gdata(10) <= atoi(words[4]));
				}
				return false;
			}
			if (equals(words[1], "month")){
				if (wordnumber == 3){
					judge(gdata(11) == atoi(words[2]));
				}
				if (wordnumber == 5){
					if (atoi(words[2]) <= atoi(words[4])){
						judge(atoi(words[2]) <= gdata(11) && gdata(11) <= atoi(words[4]));
					}
					else {
						judge(gdata(11) <= atoi(words[2]) || atoi(words[4]) <= gdata(11));
					}
				}
				return false;
			}
			if (equals(words[1], "date")){
				if (wordnumber == 3){
					judge(gdata(11) == atoi(words[2]));
				}
				if (wordnumber == 5){
					if (atoi(words[2]) <= atoi(words[4])){
						judge(atoi(words[2]) <= gdata(12) && gdata(12) <= atoi(words[4]));
					}
					else {
						judge(gdata(12) <= atoi(words[2]) || atoi(words[4]) <= gdata(12));
					}
				}
				return false;
			}
			if (equals(words[1], "hour")){
				if (wordnumber == 3){
					judge(gdata(11) == atoi(words[2]));
				}
				if (wordnumber == 5){
					if (atoi(words[2]) <= atoi(words[4])){
						judge(atoi(words[2]) <= gdata(13) && gdata(13) <= atoi(words[4]));
					}
					else {
						judge(gdata(13) <= atoi(words[2]) || atoi(words[4]) <= gdata(13));
					}
				}
				return false;
			}
			if (equals(words[1], "time")){
				if (wordnumber == 3){
					char *t[2];
					if (split(words[2], ":", t, 2) != 2) return false;
					judge(atoi(t[0]) * 24 + atoi(t[1]) == gdata(13) * 24 + gdata(14));
				}
				if (wordnumber == 5){
					char *ft[2], *tt[2];
					if (split(words[2], ":", ft, 2) != 2) return false;
					if (split(words[4], ":", tt, 2) != 2) return false;
					if (atoi(ft[0]) * 24 + atoi(ft[1]) <= atoi(tt[0]) * 24 + atoi(tt[1])){
						judge(atoi(ft[0]) * 24 + atoi(ft[1]) <= gdata(13) * 24 + gdata(14) && gdata(13) * 24 + gdata(14) <= atoi(tt[0]) * 24 + atoi(tt[1]));
					}
					else{
						judge(gdata(13) * 24 + gdata(14) <= atoi(ft[0]) * 24 + atoi(ft[1]) || atoi(tt[0]) * 24 + atoi(tt[1]) <= gdata(13) * 24 + gdata(14));
					}
				}
				return false;
			}
			if (equals(words[1], "Midnight")){
				judge(0 <= gdata(13) && gdata(13) <= 3);
			}
			if (equals(words[1], "Dawn")){
				judge(4 <= gdata(13) && gdata(13) <= 7);
			}
			if (equals(words[1], "Morning")){
				judge(8 <= gdata(13) && gdata(13) <= 11);
			}
			if (equals(words[1], "Noon")){
				judge(12 <= gdata(13) && gdata(13) <= 15);
			}
			if (equals(words[1], "Dusk")){
				judge(16 <= gdata(13) && gdata(13) <= 19);
			}
			if (equals(words[1], "Night")){
				judge(20 <= gdata(13) && gdata(13) <= 23);
			}
			return false;
		}
		if (equals(words[0], "where")){
			if (filterenable && strstr(data.filter, "/where/") != NULL) return false;
			if (equals(words[1], "North_Tyris")){
				judge(gdata(20) == 4);
			}
			if (equals(words[1], "Home")){
				judge(gdata(20) == 7);
			}
			if (equals(words[1], "Party")){
				judge(gdata(20) == 13 && gdata(70) == 1009);
			}
			if (equals(words[1], "Outskirts")){
				judge(gdata(20) == 13 && gdata(70) == 1001);
			}
			if (equals(words[1], "Urban_Area")){
				judge(gdata(20) == 13 && (gdata(70) == 1008 || gdata(70) == 1010));
			}
			if (equals(words[1], "Crop")){
				judge(gdata(20) == 13 && gdata(70) == 1006);
			}
			if (equals(words[1], "Town")){
				judge(gdata(20) == 5 || gdata(20) == 11 || gdata(20) == 12 || gdata(20) == 14 || gdata(20) == 15 || gdata(20) == 36 || gdata(20) == 33 || gdata(20) == 25);
			}
			if (equals(words[1], "Vernis")){
				judge(gdata(20) == 5);
			}
			if (equals(words[1], "Port_Kapul")){
				judge(gdata(20) == 11);
			}
			if (equals(words[1], "Yowyn")){
				judge(gdata(20) == 12);
			}
			if (equals(words[1], "Derphy")){
				judge(gdata(20) == 14);
			}
			if (equals(words[1], "Palmia")){
				judge(gdata(20) == 15);
			}
			if (equals(words[1], "Lumiest")){
				judge(gdata(20) == 36);
			}
			if (equals(words[1], "Noyel")){
				judge(gdata(20) == 33);
			}
			if (equals(words[1], "Larna")){
				judge(gdata(20) == 25);
			}
			if (equals(words[1], "Cyber_Dome")){
				judge(gdata(20) == 21);
			}
			if (equals(words[1], "Pet_Arena")){
				judge(gdata(20) == 40);
			}
			if (equals(words[1], "Truce_Ground")){
				judge(gdata(20) == 20);
			}
			if (equals(words[1], "Graveyard")){
				judge(gdata(20) == 10);
			}
			if (equals(words[1], "Embassy")){
				judge(gdata(20) == 32);
			}
			if (equals(words[1], "Museum")){
				judge(gdata(20) == 101);
			}
			if (equals(words[1], "Shop")){
				judge(gdata(20) == 102);
			}
			if (equals(words[1], "Storage_House")){
				judge(gdata(20) == 104);
			}
			if (equals(words[1], "Ranch")){
				judge(gdata(20) == 31);
			}
			if (equals(words[1], "Shelter")){
				judge(gdata(20) == 30);
			}
			if (equals(words[1], "Sister")){
				judge(gdata(20) == 29);
			}
			if (equals(words[1], "Pyramid")){
				judge(gdata(20) == 37);
			}
			if (equals(words[1], "Jail")){
				judge(gdata(20) == 41);
			}
			if (equals(words[1], "Mountain_Pass")){
				judge(gdata(20) == 26);
			}
			if (equals(words[1], "Wilderness")){
				judge(gdata(20) == 2);
			}
			if (equals(words[1], "Show_House")){
				judge(gdata(20) == 35);
			}
			if (equals(words[1], "Lesimas")){
				judge(gdata(20) == 3);
			}
			if (equals(words[1], "Void")){
				judge(gdata(20) == 42);
			}
			if (equals(words[1], "Nefia")){
				judge(450 <= gdata(20) && gdata(20) <= 500);
			}
			if (equals(words[1], "floor")) {
				int floor = 0;
				if (gdata(20) == 7 && gdata(22) != 1) {
					floor = (gdata(22) > 1) ? -(gdata(22) - 1) : -(gdata(22) - 2);
				}
				if ((adata(0, gdata(20)) != 3) && (adata(16, gdata(20)) == 3 || adata(16, gdata(20)) == 8 || adata(16, gdata(20)) == 13 || (mdata(6) >= 20 && mdata(6) <= 23) == 1))  {
					floor = gdata(22) - adata(17, gdata(20)) + 1;
				}
				if (wordnumber == 3) {
					if (words[2][0] == 'B') words[2][0] = '-';
					judge(atoi(words[2]) == floor);
				}
				if (wordnumber == 4) {
					if (equals(words[3], "-")) {
						if (words[2][0] == 'B') words[2][0] = '-';
						judge(atoi(words[2]) <= floor);
					}
					if (equals(words[2], "-")) {
						if (words[3][0] == 'B') words[3][0] = '-';
						judge(floor <= atoi(words[3]));
					}
				}
				if (wordnumber == 5) {
					if (words[2][0] == 'B') words[2][0] = '-';
					if (words[4][0] == 'B') words[4][0] = '-';
					judge(atoi(words[2]) <= floor && floor <= atoi(words[4]));
				}
				return false;
			}
			return false;
		}
		if (equals(words[0], "weather")){
			if (filterenable && strstr(data.filter, "/weather/") != NULL) return false;
			if (equals(words[1], "Sun")){
				judge(gdata(17) == 0);
			}
			if (equals(words[1], "Rain")){
				judge(gdata(17) == 3);
			}
			if (equals(words[1], "Snow")){
				judge(gdata(17) == 2);
			}
			if (equals(words[1], "Hard_rain")){
				judge(gdata(17) == 4);
			}
			if (equals(words[1], "Etherwind")){
				judge(gdata(17) == 1);
			}
			return false;
		}
		break;
	default:
		break;
	}
	return false;
};

bool varticalbar(char condition[], args data){
	char *conditions[CONDITION_NUMBER_LIMIT];
	int conditionnumber = split(condition, "|", conditions, CONDITION_NUMBER_LIMIT);

	for (int i = 0; i < conditionnumber; i++){
		if (judgement(conditions[i], data)) return true; //縦棒で区切られた条件のどれか一つでもtrueならtrueを返す(短絡評価)
	}
	return false; //縦棒で区切られた条件がすべてfalseならfalseを返す
}

bool comma(char condition[], args data){
	char *conditions[CONDITION_NUMBER_LIMIT];
	int conditionnumber = split(condition, ",", conditions, CONDITION_NUMBER_LIMIT);

	for (int i = 0; i < conditionnumber; i++){
		if (!varticalbar(conditions[i], data)) return false; //コンマで区切られた条件のどれか一つでもfalseならfalseを返す(短絡評価)
	}
	return true; //コンマで区切られた条件がすべてtrueならtrueを返す
}

bool slash(char condition[], args data){
	char *conditions[CONDITION_NUMBER_LIMIT];
	int conditionnumber = split(condition, "/", conditions, CONDITION_NUMBER_LIMIT);

	for (int i = 0; i < conditionnumber; i++){
		if (comma(conditions[i], data)) return true; //スラッシュで区切られた条件のどれか一つでもtrueならtrueを返す(短絡評価)
	}
	return false; //スラッシュで区切られた条件がすべてfalseならfalseを返す
}

EXPORT int addt(HSPEXINFO *hei){

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

	char *lines[LINE_NUMBER_LIMIT];
	int linenumber = split(data.buff, "\n", lines, LINE_NUMBER_LIMIT); // \r(改行コードCR部)が残っているが必要なときの処理で十分

	for (int i = 0; i < linenumber; i++){
		nest = removefs(lines[i]);
		if (lines[i][0] == '/' && lines[i][1] == '/') continue;
		if ((lines[i][0] == '$') || ((lines[i][0] == '%') && (lines[i][1] == '$'))) {
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

			if (rankmax <= rank){
				if (slash(lines[i], data)){
					nestlim = -2;
				}
				else {
					nestlim = nest - 1;
				}
			}
			else {
				nestlim = nest - 1;
			}

			if (nestlim == -2 && rankmax < rank) {
				rankmax = rank;
				strcpy(builder, "\r\n"); // \r(改行コードCR部)を足す必要がある
				added = false;
			}
		}
		else {
			if (nestlim == (-2) || nest <= nestlim){
				if (added) strcat(builder, "\n");
				strcat(builder, lines[i]);
				added = true;
			}
		}
	}

	data.buff = strcpy(data.buff, builder);
	return 0;
}

EXPORT int txtmid(char *buff, char *section){
	char *builder;

	if (strstr(buff, section) == NULL) {
		strcpy(buff, "");
		return 0;
	}
	else {
		builder = strstr(buff, section);
	}

	for (int i = 1; true; i++){
		if (builder[i + 1] == '\0') break;
		if (builder[i - 1] == '\n' && builder[i] == '%' && builder[i + 1] != '$') {
			builder[i - 1] = '\0';
			break;
		}
	}

	strcpy(buff, builder);
	return 0;
}

EXPORT int countchar(char *buff, char *target){
	int count = 0;

	for (int i = 0; buff[i] != '\0'; i++){
		if (buff[i] == target[0]) count++;
	}

	return count;
}