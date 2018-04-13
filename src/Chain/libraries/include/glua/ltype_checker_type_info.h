#ifndef ltype_checker_type_info_h
#define ltype_checker_type_info_h

#include <glua/lprefix.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <list>
#include <vector>
#include <unordered_set>
#include <queue>
#include <memory>
#include <functional>
#include <algorithm>

#include <glua/llimits.h>
#include <glua/lstate.h>		  
#include <glua/lua.h>
#include <glua/thinkyoung_lua_api.h>
#include <glua/glua_tokenparser.h>
#include <glua/lparsercombinator.h>
#include <glua/exceptions.h>
#include <glua/glua_debug_file.h>
#include <glua/glua_lutil.h>

namespace glua {
	namespace parser {

		typedef ::GluaTypeInfoEnum GluaTypeInfoEnum;

		struct GluaTypeInfo;


		typedef GluaTypeInfo* GluaTypeInfoP;

		struct GluaTypeInfo
		{
			GluaTypeInfoEnum etype;

			// function type fields
			std::vector<std::string> arg_names;
			// �����������ѭ�����ã�����Ƕ�׺�����������object���ͣ�������GluaTypeChecker�м�¼����������Ϣ�Ķ���Ȼ��ʹ��GluaTypeInfo *
			std::vector<GluaTypeInfoP> arg_types;
			std::vector<GluaTypeInfoP> ret_types;
			bool is_offline;
			bool declared;
			bool is_any_function;
			bool is_any_contract; // ��Ϊ��Լʹ�ã�����������������table
			bool is_stream_type; // �Ƿ��Ƕ����������ͣ�Ҳ����������֮һ

			bool is_literal_token_value; // �Ƿ��ǵ�token������ֵ
			GluaParserToken literal_value_token; // ����ǵ�token���ʽ������洢����ֵ��token

			// end function type fields

			// record���͵ĸ�����
			std::unordered_map<std::string, GluaTypeInfoP> record_props;
			std::unordered_map<std::string, std::string> record_default_values; // record���Ե�Ĭ��ֵ
			std::string record_name; // record�������ƣ��������typedef���������������
			std::string record_origin_name; // recordԭʼ���ƣ�����õ����ͣ�������Ƿ�����ȫչ�����������ƣ�����G1<G2<Person, string>, string, int>
			std::vector<GluaTypeInfoP> record_generics; // record�������õ��ķ���
			std::vector<GluaTypeInfoP> record_all_generics; // record���ʹ���ʱ�����з��Ͳ���
			std::vector<GluaTypeInfoP> record_applied_generics; // record���ʹ�����ʵ���������з��Ͳ���

			// �������͵�����
			std::string generic_name; // ��������

			// �б����͵�����
			GluaTypeInfoP array_item_type; // �б������е�ÿһ�������

			// Map���͵�����
			GluaTypeInfoP map_item_type; // Map�����е�ֵ����
			bool is_literal_empty_table; // �Ƿ�յ�������Array/Map����

			// literal type������
			std::vector<GluaParserToken> literal_type_options; // literal type�Ŀ�ѡ��ֵ

			std::unordered_set<GluaTypeInfoP> union_types; // may be any one of these types, not supported nested full info function now

			GluaTypeInfo(GluaTypeInfoEnum type_info_enum = GluaTypeInfoEnum::LTI_OBJECT);

			bool is_contract_type() const;

			bool is_function() const;

			bool is_int() const;

			bool is_number() const;

			bool is_bool() const;

			bool is_string() const;

			// ��literal type�еĿ��԰�����ÿһ�������
			bool is_literal_item_type() const;

			// TODO: ��__call������__��ͷ��������������Ԫ��������ʹ���ڲ�������ʱҪ�����غ������ж�����

			bool has_call_prop() const;

			bool may_be_callable() const;

			bool is_nil() const;

			bool is_undefined() const;

			bool is_union() const;

			bool is_record() const;

			bool is_array() const;

			bool is_map() const;
				
			bool is_table() const;

			// �����table���ͣ�etype��LTI_TABLE����
			bool is_narrow_table() const;

			// ����������table�����ͣ�����table, record, Map, Array
			bool is_like_table() const;
				
			bool is_generic() const;

			bool is_literal_type() const;

			// �жϺ������Ƿ���...�������Ƿ������������)
			bool has_var_args() const;

			// ��Ԫ����(__��ͷ�ĳ�Ա���������Զ�����record���͵�metatable��)
			bool has_meta_method() const;

			bool is_same_record(GluaTypeInfoP other) const;

			size_t min_args_count_require() const;

			// ���literal type�������Ƿ�ƥ����ֵ������
			bool match_literal_type(GluaTypeInfoP value_type) const;
			// ���literal type����ֵ�ǵ�token��������������Ƿ�ƥ��
			bool match_literal_value(GluaParserToken value_token) const;

			// ���literal type���Ƿ���ĳ������value_type����
			bool contains_literal_item_type(GluaTypeInfoP value_type) const;

			// @param show_record_details �Ƿ���ʾrecord���͵���ϸ������Ϊ�˱���ݹ���ѭ��
			std::string str(const bool show_record_details = false) const;

			// �Ѻ�Լ��storage������Ϣ����gluaģ������
			// @throws LuaException
			bool put_contract_storage_type_to_module_stream(GluaModuleByteStreamP stream);

			// �Ѻ�Լ��storage��APIs������Ϣ����gluaģ������
			// @throws LuaException
			bool put_contract_apis_info_to_module_stream(GluaModuleByteStreamP stream);

		};

	} // end namespace glua::parser
}

#endif
