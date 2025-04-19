#ifndef _BASE_EXPRESSION_HPP
#define _BASE_EXPRESSION_HPP

#include <algorithm>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>

#include <fmt/format.h>

namespace base
{
class Formula : public std::enable_shared_from_this<Formula>
{
private:
    static unsigned int generateId();

protected:
    unsigned int id_;
    std::string name_;
    std::string typeName_;

    Formula(std::string name, std::string typeName);

public:
    template <typename Derived>
    std::shared_ptr<Derived> getPtr()
    {
        static_assert(
            std::is_base_of_v<Formula, Derived>,
            "Derived must be a subclass of Formula!"
        );

        std::shared_ptr<Derived> ptr = std::dynamic_pointer_cast<Derived>(shared_from_this());

        if (!ptr)
        {
            throw std::runtime_error(
                fmt::format(
                    "Engine base expression: "
                    "Error trying to downcast \"{}\" to \"{}\" from a formula of type \"{}\".",
                    typeid(Formula).name(),
                    typeid(Derived).name(),
                    typeid(decltype(*shared_from_this())).name()
                )
            );
        }

        return ptr;
    }

    virtual ~Formula();

    virtual bool isTerm() const;

    virtual bool isOperation() const;

    virtual bool isAnd() const;

    virtual bool isOr() const;

    virtual bool isChain() const;

    virtual bool isImplication() const;

    virtual bool isBroadcast() const;

    unsigned int getId() const;

    std::string getTypeName() const;

    std::string getName() const;
};

template <typename T>
class Term : public Formula
{
private:
    T fn_;

protected:
    Term(std::string name, T fn)
        : Formula(name, "Term")
        , fn_{fn}
    {}

    template <typename U>
    friend std::shared_ptr<Term<U>> make_term(std::string, U fn);

public:

    [[nodiscard]] static std::shared_ptr<Term> create(std::string name, T fn)
    {
        return std::shared_ptr<Term>(name, fn);
    }

    virtual ~Term() = default;

    bool isTerm() const override { return true; }

    T getFn() const { return fn_; }

    void setFn(T fn) { fn_ = fn; }
};

class Operation : public Formula
{

protected:
    std::vector<std::shared_ptr<Formula>> operands_;

    Operation(
        std::string name,
        std::string nameType,
        std::vector<std::shared_ptr<Formula>> operands);

public:
    
    virtual ~Operation();

    bool isOperation() const override;

    const std::vector<std::shared_ptr<Formula>>& getOperands() const;

    std::vector<std::shared_ptr<Formula>>& getOperands();
};

class Implication : public Operation
{
public:
    [[nodiscard]] static std::shared_ptr<Implication>
    create(std::string name, std::shared_ptr<Formula> leftOp, std::shared_ptr<Formula> rightOp)
    {
        return std::shared_ptr<Implication>(new Implication(name, leftOp, rightOp));
    }

    virtual ~Implication();

    bool isImplication() const override;

protected:
    Implication(std::string name,
                std::shared_ptr<Formula> leftOp,
                std::shared_ptr<Formula> rightOp);
};


class And : public Operation
{
public:
    [[nodiscard]] static std::shared_ptr<And>
    create(std::string name, std::vector<std::shared_ptr<Formula>> operands)
    {
        return std::shared_ptr<And>( new And(name, operands));
    }

    virtual ~And();

    bool isAnd() const override;

protected:
    
    And(std::string name, std::vector<std::shared_ptr<Formula>> operands);
};

class Or : public Operation
{
public:
    [[nodiscard]] static std::shared_ptr<Or>
    create(std::string name, std::vector<std::shared_ptr<Formula>> operands)
    {
        return std::shared_ptr<Or>(new Or(name, operands));
    }

    virtual ~Or();

    bool isOr() const override;

protected:
    Or(std::string name, std::vector<std::shared_ptr<Formula>> operands);
};

class Chain : public Operation
{
public:
    [[nodiscard]] static std::shared_ptr<Chain>
    create (std::string name, std::vector<std::shared_ptr<Formula>> operands)
    {
        return std::shared_ptr<Chain>(new Chain(name, operands));
    }

    virtual ~Chain();

    bool isChain() const override;

protected:
    Chain(std::string name, std::vector<std::shared_ptr<Formula>> operands);
};

class Broadcast : public Operation
{
public:
    [[nodiscard]] static std::shared_ptr<Broadcast>
    create(std::string name, std::vector<std::shared_ptr<Formula>> operands)
    {
        return std::shared_ptr<Broadcast>(new Broadcast(name, operands));
    }

    virtual ~Broadcast();

    bool isBroadcast() const override;

protected:
    
    Broadcast(std::string name, std::vector<std::shared_ptr<Formula>> operands);
};

using Expression = std::shared_ptr<Formula>;

std::string toGraphvizStr(Expression expression);

} // namespace base

#endif //  _BASE_EXPRESSION_HPP
