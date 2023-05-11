// ExclusionGroup.h
//
// A class that handles enhancements which need to be exclusive across multiple
// enhancement trees. To avoid having to update all trees when a new tree is 
// added, enhancements will check to see if they are in the named Exclusion Group
// If they are, they can be trained, if not they can't. E.g. Efficient Metamagic "Maximize"
// can only be trained once across all trees so an enhancement will be setup
// to use exclusion group "Maximize"
#pragma once

class ExclusionGroup
{
    public:
        ExclusionGroup(const std::string& name, const std::string& enhancementId);
        virtual ~ExclusionGroup();

        const std::string& Name() const;
        const std::string& EnhancementId() const;
        void AddStack();
        bool RevokeStack(); // returns true if last stack

    private:
        std::string m_name;
        std::string m_enhancementId;
        size_t m_count;
};
