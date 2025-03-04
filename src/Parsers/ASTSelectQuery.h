#pragma once

#include <Parsers/IAST.h>
#include <Core/Names.h>


namespace DB
{

struct ASTTablesInSelectQueryElement;
struct StorageID;


/** SELECT query
  */
class ASTSelectQuery : public IAST
{
public:
    enum class Expression : uint8_t
    {
        WITH,
        SELECT,
        TABLES,
        PREWHERE,
        WHERE,
        /// proton: starts
        PARTITION_BY, /// Partition the data and track watermark for each partitioned data (substream) separately.
        SHUFFLE_BY, /// shuffle by is light-weight patition by without calculating substream ID which is 128 bits long.
        /// proton: ends
        GROUP_BY,
        HAVING,
        WINDOW,
        ORDER_BY,
        LIMIT_BY_OFFSET,
        LIMIT_BY_LENGTH,
        LIMIT_BY,
        LIMIT_OFFSET,
        LIMIT_LENGTH,
        /// proton: starts
        EMIT,
        /// proton: ends
        SETTINGS
    };

    static String expressionToString(Expression expr)
    {
        switch (expr)
        {
            case Expression::WITH:
                return "WITH";
            case Expression::SELECT:
                return "SELECT";
            case Expression::TABLES:
                return "TABLES";
            case Expression::PREWHERE:
                return "PREWHERE";
            case Expression::WHERE:
                return "WHERE";
            /// proton: starts
            case Expression::PARTITION_BY:
                return "PARTITION BY";
            case Expression::SHUFFLE_BY:
                return "SHUFFLE BY";
            /// proton: ends
            case Expression::GROUP_BY:
                return "GROUP BY";
            case Expression::HAVING:
                return "HAVING";
            case Expression::WINDOW:
                return "WINDOW";
            case Expression::ORDER_BY:
                return "ORDER BY";
            case Expression::LIMIT_BY_OFFSET:
                return "LIMIT BY OFFSET";
            case Expression::LIMIT_BY_LENGTH:
                return "LIMIT BY LENGTH";
            case Expression::LIMIT_BY:
                return "LIMIT BY";
            case Expression::LIMIT_OFFSET:
                return "LIMIT OFFSET";
            case Expression::LIMIT_LENGTH:
                return "LIMIT LENGTH";
            /// proton: starts
            case Expression::EMIT:
                return "EMIT";
            /// proton: ends
            case Expression::SETTINGS:
                return "SETTINGS";
        }
        return "";
    }

    /** Get the text that identifies this element. */
    String getID(char) const override { return "SelectQuery"; }

    ASTPtr clone() const override;

    bool distinct = false;
    bool group_by_with_totals = false;
    bool group_by_with_rollup = false;
    bool group_by_with_cube = false;
    bool group_by_with_constant_keys = false;
    bool group_by_with_grouping_sets = false;
    bool limit_with_ties = false;

    ASTPtr & refSelect()    { return getExpression(Expression::SELECT); }
    ASTPtr & refTables()    { return getExpression(Expression::TABLES); }
    ASTPtr & refPrewhere()  { return getExpression(Expression::PREWHERE); }
    ASTPtr & refWhere()     { return getExpression(Expression::WHERE); }
    ASTPtr & refHaving()    { return getExpression(Expression::HAVING); }

    ASTPtr with()           const { return getExpression(Expression::WITH); }
    ASTPtr select()         const { return getExpression(Expression::SELECT); }
    ASTPtr tables()         const { return getExpression(Expression::TABLES); }
    ASTPtr prewhere()       const { return getExpression(Expression::PREWHERE); }
    ASTPtr where()          const { return getExpression(Expression::WHERE); }
    /// proton: starts
    ASTPtr partitionBy()    const { return getExpression(Expression::PARTITION_BY); }
    ASTPtr shuffleBy()      const { return getExpression(Expression::SHUFFLE_BY); }
    /// proton: ends
    ASTPtr groupBy()        const { return getExpression(Expression::GROUP_BY); }
    ASTPtr having()         const { return getExpression(Expression::HAVING); }
    ASTPtr window() const { return getExpression(Expression::WINDOW); }
    ASTPtr orderBy()        const { return getExpression(Expression::ORDER_BY); }
    ASTPtr limitByOffset()  const { return getExpression(Expression::LIMIT_BY_OFFSET); }
    ASTPtr limitByLength()  const { return getExpression(Expression::LIMIT_BY_LENGTH); }
    ASTPtr limitBy()        const { return getExpression(Expression::LIMIT_BY); }
    ASTPtr limitOffset()    const { return getExpression(Expression::LIMIT_OFFSET); }
    ASTPtr limitLength()    const { return getExpression(Expression::LIMIT_LENGTH); }
    /// proton: starts
    ASTPtr emit()           const { return getExpression(Expression::EMIT); }
    /// proton: ends
    ASTPtr settings()       const { return getExpression(Expression::SETTINGS); }

    bool hasFiltration() const { return where() || prewhere() || having(); }

    /// Set/Reset/Remove expression.
    void setExpression(Expression expr, ASTPtr && ast);

    ASTPtr getExpression(Expression expr, bool clone = false) const
    {
        auto it = positions.find(expr);
        if (it != positions.end())
            return clone ? children[it->second]->clone() : children[it->second];
        return {};
    }

    /// Compatibility with old parser of tables list. TODO remove
    ASTPtr sampleSize() const;
    ASTPtr sampleOffset() const;
    std::pair<ASTPtr, bool> arrayJoinExpressionList() const;

    const ASTTablesInSelectQueryElement * join() const;
    bool final() const;
    bool withFill() const;
    void replaceDatabaseAndTable(const String & database_name, const String & table_name);
    void replaceDatabaseAndTable(const StorageID & table_id);
    void addTableFunction(ASTPtr & table_function_ptr);
    void updateTreeHashImpl(SipHash & hash_state) const override;

    void setFinal();

    virtual QueryKind getQueryKind() const override { return QueryKind::Select; }

    /// proton: starts. HACK
    void setSubQueryPipe(const std::string & subquery) { subquery_pipe = subquery; }
    /// proton: ends

protected:
    void formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const override;

private:
    /// proton: starts. HACK
    String subquery_pipe;
    /// proton: ends

    std::unordered_map<Expression, size_t> positions;

    ASTPtr & getExpression(Expression expr);
};

}
