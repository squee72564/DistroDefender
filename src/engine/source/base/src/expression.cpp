#include <atomic>

#include <base/expression.hpp>

namespace base
{

using Expression = std::shared_ptr<Formula>;

unsigned int Formula::generateId()
{
    static std::atomic<unsigned int> id{0};
    return id++;
}

Formula::Formula(std::string name, std::string typeName)
    : id_{generateId()}
    , name_{std::move(name)}
    , typeName_{std::move(typeName)} {}

Formula::~Formula() = default;

bool Formula::isTerm() const
{
    return false;
}

bool Formula::isOperation() const
{
    return false;
}

bool Formula::isAnd() const
{
    return false;
}

bool Formula::isOr() const
{
    return false;
}

bool Formula::isChain() const
{
    return false;
}


bool Formula::isImplication() const
{
    return false;
}

bool Formula::isBroadcast() const
{
    return false;
}

unsigned int Formula::getId() const
{
    return id_;
}

std::string Formula::getTypeName() const
{
    return typeName_;
}

std::string Formula::getName() const
{
    return name_;
}

Operation::Operation(
    std::string name,
    std::string nameType,
    std::vector<std::shared_ptr<Formula>> operands)
    : Formula(name, nameType)
    , operands_{operands}
{}

Operation::~Operation() = default;

bool Operation::isOperation() const
{
    return true;
}

const std::vector<std::shared_ptr<Formula>>& Operation::getOperands() const
{
    return operands_;
}

std::vector<std::shared_ptr<Formula>>& Operation::getOperands()
{
    return operands_;
}

Implication::~Implication() = default;

bool Implication::isImplication() const
{
    return true;
}

Implication::Implication(std::string name, std::shared_ptr<Formula> leftOp, std::shared_ptr<Formula> rightOp)
    : Operation{name, "Implication", {leftOp, rightOp}}
{}

And::~And() = default;

bool And::isAnd() const
{
    return true;
}

And::And(std::string name, std::vector<std::shared_ptr<Formula>> operands)
    : Operation{name, "And", operands}
{}

Or::~Or() = default;

bool Or::isOr() const
{
    return true;
}

Or::Or(std::string name, std::vector<std::shared_ptr<Formula>> operands)
    : Operation{name, "Or", operands}
{}

Chain::~Chain() = default;

bool Chain::isChain() const
{
    return true;
}

Chain::Chain(std::string name, std::vector<std::shared_ptr<Formula>> operands)
    : Operation{name, "Chain", operands}
{}

Broadcast::~Broadcast() = default;

bool Broadcast::isBroadcast() const
{
    return true;
}

Broadcast::Broadcast(std::string name, std::vector<std::shared_ptr<Formula>> operands)
    : Operation(name, "Broadcast", operands)
{}

std::string toGraphvizStr(Expression expression)
{
    auto header = R"(
    compound=true;
    fontname="Helvetica,Arial,sans-serif";
    fontsize=12;
    node [color="#57abff", fontname="Helvetica,Arial,sans-serif", fontsize=10, fontcolor="white"];
    edge [fontname="Helvetica,Arial,sans-serif", fontsize=8];
    )";
    auto clusterHeader = R"(
    style="rounded,filled";
    color="#57abff";
    )";

    std::stringstream ss;
    ss << "strict digraph G {" << std::endl << header << std::endl;

    // Erase " from string
    auto eraseQuotes = [](std::string str)
    {
        while (str.find("\"") != std::string::npos)
        {
            str.replace(str.find("\""), 1, "'");
        }

        return str;
    };
    auto visit = [&](Expression current, auto& visitRef) -> void
    {
        ss << "subgraph cluster_" << current->getId() << " {" << std::endl << clusterHeader << std::endl;
        ss << "label=\"" << current->getTypeName() << "\";" << std::endl;
        ss << current->getId() << " [label=\"" << eraseQuotes(current->getName()) << " [" << current->getId() << "]\"];"
           << std::endl;
        ss << "}" << std::endl;
        if (current->isOperation())
        {
            auto operation = current->getPtr<Operation>();
            int i = 0;
            for (auto& child : operation->getOperands())
            {
                if (child)
                {
                    ss << fmt::format("{} -> {} [ltail=cluster_{} lhead=cluster_{} label={} "
                                      "fontcolor=\"red\"];\n",
                                      current->getId(),
                                      child->getId(),
                                      current->getId(),
                                      child->getId(),
                                      i++);
                    visitRef(child, visitRef);
                }
            }
        }
    };
    visit(expression, visit);
    ss << "}\n";
    return ss.str();
}

} // namespace base
