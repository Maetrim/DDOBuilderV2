// Attack.h
//
// An XML object wrapper that holds information on which equipment slot(s)
// and item can be equipped to.
#pragma once
#include "XmlLib\DLMacros.h"
#include "AttackBonus.h"

class Attack :
    public XmlLib::SaxContentElement
{
    public:
        Attack(void);
        Attack(const std::string& name, const std::string& description, const std::string& icon);
        void Write(XmlLib::SaxWriter * writer) const;

        size_t Stacks() const;
        void AddStack();
        void RevokeStack();

        void SetCooldown(double cooldown);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Attack_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Description) \
                DL_STRING(_, Icon) \
                DL_OPTIONAL_VECTOR(_, double, Cooldown) \
                DL_OPTIONAL_OBJECT(_, AttackBonus, ThisAttack) \
                DL_OPTIONAL_OBJECT(_, AttackBonus, FollowOn)

        DL_DECLARE_ACCESS(Attack_PROPERTIES)
        DL_DECLARE_VARIABLES(Attack_PROPERTIES)
    private:
        int m_stacks;
};
