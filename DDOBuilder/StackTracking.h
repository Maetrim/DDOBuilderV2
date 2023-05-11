// StackTracking.h
//
// A class that handles list of weapons that belong to a specific grouping
// grouping will be things like "Light Weapons", "Favored Weapons", "Focus Weapons"
#pragma once

class StackTracking
{
    public:
        StackTracking(const std::string& name);
        virtual ~StackTracking();

        const std::string& Name() const;
        size_t Stacks() const;
        void AddStack();
        void RevokeStack();

    private:
        std::string m_name;
        size_t m_stacks;
};
