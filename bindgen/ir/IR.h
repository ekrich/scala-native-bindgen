#ifndef SCALA_NATIVE_BINDGEN_INTERMEDIATEREPRESENTATION_H
#define SCALA_NATIVE_BINDGEN_INTERMEDIATEREPRESENTATION_H

#include "Enum.h"
#include "Function.h"
#include "LiteralDefine.h"
#include "LocationManager.h"
#include "PossibleVarDefine.h"
#include "Struct.h"
#include "TypeDef.h"
#include "VarDefine.h"

/**
 * Intermediate representation
 */
class IR {
  public:
    IR(std::string libName, std::string linkName, std::string objectName,
       std::string packageName, const LocationManager &locationManager);

    ~IR();

    void addFunction(std::string name, std::vector<Parameter *> parameters,
                     std::shared_ptr<Type> retType, bool isVariadic);

    std::shared_ptr<TypeDef> addTypeDef(std::string name,
                                        std::shared_ptr<Type> type,
                                        std::shared_ptr<Location> location);

    /**
     * @return type alias for the enum
     */
    void addEnum(std::string name, const std::string &type,
                 std::vector<Enumerator> enumerators,
                 std::shared_ptr<Location> location);

    void addStruct(std::string name, std::vector<Field *> fields,
                   uint64_t typeSize, std::shared_ptr<Location> location);

    void addUnion(std::string name, std::vector<Field *> fields,
                  uint64_t maxSize, std::shared_ptr<Location> location);

    void addLiteralDefine(std::string name, std::string literal,
                          std::shared_ptr<Type> type);

    void addPossibleVarDefine(const std::string &macroName,
                              const std::string &varName);

    void addVarDefine(std::string name, std::shared_ptr<Variable> variable);

    std::shared_ptr<Variable> addVariable(const std::string &name,
                                          std::shared_ptr<Type> type);

    /**
     * @return true if there are no functions, types,
     *         structs and unions
     */
    bool libObjEmpty() const;

    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const IR &ir);

    void generate(const std::string &excludePrefix);

    void removeDefine(const std::string &name);

    /**
     * @return macro name if there is a macro for the variable.
     *         otherwise return empty string
     */
    std::string getDefineForVar(const std::string &varName) const;

    std::shared_ptr<TypeDef> getTypeDefWithName(const std::string &name) const;

  private:
    /**
     * @return true if helper methods will be generated for this library
     */
    bool hasHelperMethods() const;

    /**
     * Remove functions that start with given prefix.
     *
     * Replace typedef by underlying type if it starts with given prefix
     * and it is used only in other typedefs.
     *
     * Example:
     * @code
     * type __int32_t = native.CInt
     * type __darwin_pid_t = __int32_t
     * type pid_t = __darwin_pid_t
     * @endcode
     *
     * Becomes:
     * @code
     * type pid_t = native.CInt
     * @endcode
     *
     */
    void filterDeclarations(const std::string &excludePrefix);

    /**
     * Remove all typedefs that start with given prefix.
     */
    void filterTypeDefs(const std::string &excludePrefix);

    /**
     * Find all typedefs that use oldType and replace it with newType.
     */
    void replaceTypeInTypeDefs(std::shared_ptr<Type> oldType,
                               std::shared_ptr<Type> newType);

    /**
     * @return true if given type is used only in typedefs.
     */
    bool typeIsUsedOnlyInTypeDefs(const std::shared_ptr<Type> &type) const;

    /**
     * @param checkRecursively if this parameter is true then the method will
     * output false if the type is used only in unused types
     * @return true if type is used in one of declarations
     */
    bool isTypeUsed(const std::shared_ptr<Type> &type,
                    bool checkRecursively = false) const;

    /**
     * @return true if type is used in one of given declarations.
     */
    template <typename T>
    bool isTypeUsed(const std::vector<T> &declarations,
                    std::shared_ptr<Type> type, bool stopOnTypeDefs) const;

    void setScalaNames();

    bool existsFunctionWithName(std::string functionName);

    template <typename T>
    void filterByPrefix(std::vector<T> &declarations,
                        const std::string &excludePrefix);

    template <typename T>
    void filterByName(std::vector<T> &declarations, const std::string &name);

    template <typename T>
    T getDeclarationWithName(const std::vector<T> &declarations,
                             const std::string &name) const;

    template <typename T> bool inMainFile(const T &type) const;

    /**
     * @tparam T Type subclass
     * @return true if type is in main file or it is used by declaration from
     *         main file.
     */
    template <typename T>
    bool isOutputted(const std::shared_ptr<T> &type) const;

    /**
     * @tparam T Struct or Union
     */
    template <typename T>
    bool hasOutputtedDeclaration(
        const std::vector<std::shared_ptr<T>> &declarations) const;

    std::string libName;    // name of the library
    std::string linkName;   // name of the library to link with
    std::string objectName; // name of Scala object
    const LocationManager &locationManager;
    std::vector<std::shared_ptr<Function>> functions;
    std::vector<std::shared_ptr<TypeDef>> typeDefs;
    std::vector<std::shared_ptr<Struct>> structs;
    std::vector<std::shared_ptr<Union>> unions;
    std::vector<std::shared_ptr<Enum>> enums;
    std::vector<std::shared_ptr<LiteralDefine>> literalDefines;
    std::vector<std::shared_ptr<PossibleVarDefine>> possibleVarDefines;
    std::vector<std::shared_ptr<VarDefine>> varDefines;
    std::vector<std::shared_ptr<Variable>> variables;
    bool generated = false; // generate type defs only once
    std::string packageName;
};

#endif // SCALA_NATIVE_BINDGEN_INTERMEDIATEREPRESENTATION_H
