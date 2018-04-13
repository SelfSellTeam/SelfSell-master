#ifndef glua_astparser_h
#define glua_astparser_h

#include <glua/lprefix.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <list>
#include <queue>
#include <memory>

#include <glua/llimits.h>
#include <glua/lstate.h>			  
#include <glua/lua.h>
#include <glua/glua_tokenparser.h>
#include <glua/ltype_checker_type_info.h>

namespace glua {
    namespace parser {

		/*
		�µ�parser��ʽ�����ֱ��״̬��expect������parsercombinator
		TODO:
		expectParser: ȷ�������������.
		���ĳ��concatParser������optional�ģ��ǲ���expect������Ӧ��expect
		����parse��ʱ��Ҫ������parser���Ƿ�optional
		ÿ��parser������concatParser����orParser���������������ÿ�ε��ö���Ҫ����һ��������֪�����ε����Ƿ�optional
		��Ҫ�������ȷ������Ϣ������firstToken
		ʹ��orParser��dispatch, ����firstToken������ԭ�е�һЩ���ԣ���Ҫ��֪�����ĸ���ʼparser
		���һ����optional��orParser��firstToken���У�ƥ���firstToken��Ӧ��parsersΨһ���������parserҲ�Ƿ�optional��
		һ����optional��concatParser��ÿһ��Ƿ�optional��
		������optional��
		��ʼparse����ʼһ���Ƿ�optional��orParser
		�����Ƿ������optional��parser���û���˵���optional��(������Ҫ)

		Whenever a parser fails, it also returns information about what it expected to parse.
		The parser combinators propagate that failure information outward, but may be absorbed if an alternative
		parser succeeds.  Only when the failure is returned from the outermost parser does it become an error. 
		At that point, the failure has information about all the alternatives at the point of failure, 
		and can be turned into a useful error message.

		The rules for propagating the failures are fairly obvious, but took some trial and error for me to discover them:

		I discard all alternative failures that matches less input than another failure, under the assumption that
		the parser that matched the most input is what the user intended.
		    The describeParser combinator is used to provide a friendly description of what an inner parser will match.  
		When the inner parser fails, if it didn��t match any input, it overrides the inner parser��s description of 
		its expectations.  However, if the inner parser fails after matching some input, then it forms an expectation
		    tree where the description is used as context for the inner parser��s failed expectations.
		Because some parsers may have alternatives that succeed with a partial match of the input, they need to also 
		return information about failed alternatives that consumed more input than the successful match.  For example,
		parsing an addition expression from ��a+�� should successfully match ��a��, and return that along with
		a failed alternative that matched ��a+�� but then failed to match a term.
		    When a sequence parser fails, it needs to look at the failed alternatives for successfully matched 
		subparsers at the beginning of the sequence, in addition to the subparser failure that caused the sequence
		to fail.  For example, if the expression parser in the previous point was wrapped in a sequence that
		expected an expression followed by the end of the input, and was applied to the same input string ��a+��,
		then it would match ��a�� before failing to match ��+�� with the end of the input.  In that case, the failed
		alternative from the expression parser made it farther than the failed end of input parser, and so that is 
		the failed expectation to return from the sequence.

		Once the failure becomes fatal, it will have information about all the possible expectations at the furthest
		unmatched point in the input, organized in a tree with the nodes providing context for the failed 
		expectations.  To describe this to the user, I use the description for each leaf, and the innermost node
		that contains all the leaves as context.

		>>> a = [ "b", "c"
		error: (1:16): found 'end of input', but was expecting operator or ']' or ',' while parsing array literal at 1:5

		*/

            /************************************************************************/
            /*
    chunk ::= block

    block ::= {stat} [retstat]

    stat ::=  ��;�� |
    varlist ��=�� explist |
    functioncall |
    label |
    break |
    goto Name |
    do block end |
    while exp do block end |
    repeat block until exp |
    if exp then block {elseif exp then block} [else block] end |
    for Name ��=�� exp ��,�� exp [��,�� exp] do block end |
    for namelist in explist do block end |
    function funcname funcbody |
    local function Name funcbody |
    local namelist [��=�� explist]

    retstat ::= return [explist] [��;��]

    label ::= ��::�� Name ��::��

    funcname ::= Name {��.�� Name} [��:�� Name]

    varlist ::= var {��,�� var}

    var ::=  Name | prefixexp ��[�� exp ��]�� | prefixexp ��.�� Name

    namelist ::= Name {��,�� Name}

    explist ::= exp {��,�� exp}

    exp ::=  nil | false | true | Numeral | LiteralString | ��...�� | functiondef |
    prefixexp | tableconstructor | exp binop exp | unop exp

    prefixexp ::= var | functioncall | ��(�� exp ��)��

    functioncall ::=  prefixexp args | prefixexp ��:�� Name args

    args ::=  ��(�� [explist] ��)�� | tableconstructor | LiteralString

    functiondef ::= function funcbody

    funcbody ::= ��(�� [parlist] ��)�� block end

    parlist ::= namelist [��,�� ��...��] | ��...��

    tableconstructor ::= ��{�� [fieldlist] ��}��

    fieldlist ::= field {fieldsep field} [fieldsep]

    field ::= ��[�� exp ��]�� ��=�� exp | Name ��=�� exp | exp

    fieldsep ::= ��,�� | ��;��

    binop ::=  ��+�� | ��-�� | ��*�� | ��/�� | ��//�� | ��^�� | ��%�� |
    ��&�� | ��~�� | ��|�� | ��>>�� | ��<<�� | ��..�� |
    ��<�� | ��<=�� | ��>�� | ��>=�� | ��==�� | ��~=�� |
    and | or

    unop ::= ��-�� | not | ��#�� | ��~��
    */
            /************************************************************************/
            enum GluaAstNodeType
            {
                LA_PROTO = 0,
                LA_BLOCK = 1,
                LA_IF_STAT = 2,
                LA_FOR_STAT = 3,
                LA_WHILE_STAT = 4,
                LA_REPEAT_STAT = 5,
                LA_FUNC_STAT = 6,
                LA_LOCAL_FUNC_STAT = 7,
                LA_LOCAL_STAT = 8,
                LA_LABEL_STAT = 9,
                LA_GOTO_STAT = 10,
                LA_EXPR = 11,
                LA_AND_EXPR = 12,
                LA_OR_EXPR = 13,
                LA_SINGLE_OP_EXPR = 14, // such as +/-/*///%/&
				LA_EMIT_STAT = 15,
				LA_RECORD_STAT = 16,
				LA_TYPEDEF_STAT = 17,
				LA_BIN_EXPR = 18
            };

			// FIXME: ����̳���MatchResult��Ϊ��ֱ����ԭ���ṹ��ʹ�ã�����һ������ȫ��д
            class GluaAstNode : public MatchResult
            {
			protected:
				enum GluaAstNodeType _type;
				MatchResult *_mr;
			public:
				inline GluaAstNode() {}
				inline virtual ~GluaAstNode() {}
				inline enum GluaAstNodeType ast_node_type() const
				{
					return _type;
				}
				inline MatchResult *mr() const
				{
					return _mr;
				}
				// �õ�����dump��mr
				inline virtual MatchResult *dump_mr(ParserContext *ctx)
				{
					return _mr;
				}

				// ��Щ��Ϊ�˼���MatchResult�ṩ�ķ�������Ϊһ��proxy
				inline virtual bool is_ast_node_type() const
				{
					return true;
				}
				inline virtual Input next_input() const {
					return _mr->next_input();
				}
				inline virtual void set_input(Input &input)
				{
					_mr->set_input(input);
				}
				inline virtual bool is_final() const {
					return _mr->is_final();
				}
				inline virtual bool is_complex() const {
					return _mr->is_complex();
				}
				inline virtual MatchResult *set_hidden(bool hidden)
				{
					return _mr->set_hidden(hidden);
				}
				inline virtual bool hidden() const
				{
					return _mr->hidden();
				}
				inline virtual MatchResult *set_hidden_replace_string(std::string value)
				{
					return _mr->set_hidden_replace_string(value);
				}
				inline virtual std::string hidden_replace_string() const
				{
					return _mr->hidden_replace_string();
				}
				inline virtual MatchResult *set_need_end_of_line(bool value)
				{
					return _mr->set_need_end_of_line(value);
				}
				inline virtual bool need_end_of_line() const
				{
					return _mr->need_end_of_line();
				}
				inline virtual size_t linenumber_after_end(MatchResult *parent_mr) const
				{
					return _mr->linenumber_after_end(parent_mr);
				}
				inline virtual void set_node_name(std::string name)
				{
					_mr->set_node_name(name);
				}
				inline virtual std::string node_name() const
				{
					return _mr->node_name();
				}
				inline virtual ComplexMatchResult *as_complex() const
				{
					return _mr->as_complex();
				}
				inline virtual FinalMatchResult *as_final() const
				{
					return _mr->as_final();
				}
				inline virtual MatchResult *set_parser(ParserFunctor *parser)
				{
					return _mr->set_parser(parser);
				}
				inline virtual ParserFunctor *parser() const
				{
					return _mr->parser();
				}
				inline virtual MatchResult *invoke_parser_post_callback()
				{
					return _mr->invoke_parser_post_callback();
				}
				inline virtual void *binding() const
				{
					return _mr->binding();
				}
				inline virtual MatchResultBindingTypeEnum binding_type() const
				{
					return _mr->binding_type();
				}
				inline virtual void set_binding(void *b)
				{
					_mr->set_binding(b);
				}
				inline virtual void set_binding_type(MatchResultBindingTypeEnum t)
				{
					_mr->set_binding_type(t);
				}
				inline virtual std::string str() const
				{
					return _mr->str();
				}
				inline virtual GluaParserToken head_token() const
				{
					return _mr->head_token();
				}
				inline virtual GluaParserToken last_token() const
				{
					return _mr->last_token();
				}
            };

            typedef GluaAstNode* GluaAstNodeP;
            typedef std::list<GluaAstNodeP> GluaAstNodeList;

			// ���﷨�ı��ʽ��ֱ����mr����ʾ������һЩ��û���ü�����������﷨�ṹ��
			class GluaMrNode : public GluaAstNode
			{
			public:
				inline GluaMrNode(MatchResult *mr, enum GluaAstNodeType type)
				{
					this->_type = type;
					this->_mr = mr;
				}

				inline virtual ~GluaMrNode(){}

			};

			class GluaAstExprNode : public GluaAstNode
			{
			public:
				inline GluaAstExprNode()
				{
					this->_type = GluaAstNodeType::LA_EXPR;
				}
				inline virtual ~GluaAstExprNode() {}
			};

			typedef GluaAstExprNode* GluaAstExprNodeP;

			class GluaAstSimpleExprNode : public GluaAstExprNode
			{
				// TODO
			public:
				inline virtual ~GluaAstSimpleExprNode() {}
			};

			class GluaAstBinExprNode : public GluaAstExprNode
			{
			private:
				GluaAstExprNodeP _left;
				GluaAstExprNodeP _right;
				MatchResult *_op_mr;
			public:
				inline GluaAstBinExprNode(MatchResult *mr, GluaAstExprNodeP left, GluaAstExprNodeP right, MatchResult *op_mr)
				{
					this->_left = left;
					this->_right = right;
					this->_op_mr = op_mr;
					this->_mr = mr;
					this->_type = GluaAstNodeType::LA_BIN_EXPR;
				}
				inline virtual ~GluaAstBinExprNode() {}
			};

			typedef std::pair<MatchResult*, MatchResult*> GluaConditionNodeInfo;

			class GluaIfStatNode : public GluaAstNode
			{
			private:
				// conditions_mrs��ÿһ����һ������mr��һ�������mr��if exp then block��elseif exp then block
				std::vector<GluaConditionNodeInfo> _conditions_mrs;
				MatchResult *_else_block_mr; // else������֧�Ĵ����mr��������nullptr
			public:
				inline GluaIfStatNode(MatchResult *mr,
					std::vector<GluaConditionNodeInfo> conditions_mrs,
					MatchResult *else_block_mr=nullptr)
				{
					this->_mr = mr;
					this->_type = GluaAstNodeType::LA_IF_STAT;
					this->_conditions_mrs = conditions_mrs;
					this->_else_block_mr = else_block_mr;
				}
				inline virtual ~GluaIfStatNode() {}
				inline const std::vector<GluaConditionNodeInfo> &condition_mrs() const
				{
					return _conditions_mrs;
				}
				inline MatchResult *else_block_mr() const
				{
					return _else_block_mr;
				}
			};

    } // end of glua::parser
}

#endif