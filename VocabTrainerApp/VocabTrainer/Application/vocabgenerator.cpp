#include "vocabgenerator.h"

#include <QStack>

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("VocabGenerator");
        return tag;
    }
}

VocabGenerator::VocabGenerator(Logger &logger)
    : m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing vocab generator..."));
    m_logger.verbose(logTag(), QStringLiteral("Constructing vocab generator done!"));
}

const VocabGeneration::GenerationResult VocabGenerator::generate(const VocabFile::Data &vocabFileData)
{
    m_logger.verbose(logTag(), QStringLiteral("Generating vocab data..."));

    VocabGeneration::GenerationResult result;
    result.data.title = vocabFileData.title;

    // Generate entries without group
    m_logger.verbose(logTag(), QStringLiteral("Generating vocab data entries..."));
    VocabGeneration::GenerateEntriesResult generateEntriesResult = handleVocabFileEntries(vocabFileData.entries);
    if (generateEntriesResult.code != Error::Code::Success)
    {
        m_logger.error(logTag(), QStringLiteral("Failed to generate vocab data entries! ErrorCode %1").arg(Error::codeToQString(generateEntriesResult.code)));
        result.code = generateEntriesResult.code;
        return result;
    }

    if (generateEntriesResult.failedEntryCount > 0)
    {
        m_logger.warning(logTag(), QStringLiteral("Skipping %1 failed entries in vocab %2").arg(QString::number(generateEntriesResult.failedEntryCount), vocabFileData.title));
    }
    result.failedEntryCount += generateEntriesResult.failedEntryCount;

    uniteEntries(result.data.entries, generateEntriesResult.entries);
    m_logger.verbose(logTag(), QStringLiteral("Generating vocab data entries done!"));

    // Generate entries under groups
    m_logger.verbose(logTag(), QStringLiteral("Generating vocab data entries under groups..."));
    for (auto const &group : vocabFileData.groups)
    {
        m_logger.verbose(logTag(), QStringLiteral("Handling group '%1'...").arg(group.title));

        if (group.entries.isEmpty())
        {
            m_logger.warning(logTag(), QStringLiteral("Skipping empty group '%1'...").arg(group.title));
            result.failedGroupCount++;
            continue;
        }

        VocabGeneration::GenerateEntriesResult generateGroupEntriesResult = handleVocabFileEntries(group.entries);
        if (generateGroupEntriesResult.code != Error::Code::Success)
        {
            m_logger.error(logTag(), QStringLiteral("Failed to generate entries for group '%1'!").arg(group.title));
            result.failedGroupCount++;
            continue;
        }

        if (generateGroupEntriesResult.failedEntryCount > 0)
        {
            m_logger.warning(logTag(), QStringLiteral("Skipped %1 faulty entries in group '%2'.").arg(QString::number(generateGroupEntriesResult.failedEntryCount), group.title));
            result.partiallySucceededGroupCount++;
        }
        result.failedEntryCount += generateGroupEntriesResult.failedEntryCount;

        int successCount = vocabFileData.groups.count() - result.failedGroupCount;
        if (!(successCount > 0))
        {
            m_logger.error(logTag(), QStringLiteral("Failed to generate any groups for vocab file '%1'").arg(vocabFileData.title));
            result.code = Error::Code::NoSucceededGroups;
            return result;
        }

        // Save new generated group entries and unite them with previous group entries
        Vocab::GroupEntries newGroupEntries;
        newGroupEntries.insert(group.title, generateGroupEntriesResult.entries);
        uniteGroups(result.data.groupEntries, newGroupEntries);
    }
    m_logger.verbose(logTag(), QStringLiteral("Generating vocab data entries under groups done!"));

    m_logger.verbose(logTag(), QStringLiteral("Generating vocab data done!"));

    return result;
}

const VocabGeneration::GenerateEntriesResult VocabGenerator::handleVocabFileEntries(const QList<VocabFile::Entry> &entries)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling vocab file entries..."));
    VocabGeneration::GenerateEntriesResult result;

    if (entries.isEmpty())
    {
        m_logger.verbose(logTag(), QStringLiteral("Nothing to handle. Returning..."));
        return result;
    }

    for (const auto &entry : entries)
    {
        VocabGeneration::GenerateVariantsResult generateVariantsResult = generateVariants(entry);
        if (generateVariantsResult.code != Error::Code::Success)
        {
            m_logger.error(logTag(), QStringLiteral("Failed to generate entry '%1 = %2'! Skipping...").arg(entry.first, entry.second));
            result.failedEntryCount++;
            continue;
        }
        uniteEntries(result.entries, generateVariantsResult.entries);
    }

    int successCount = entries.count() - result.failedEntryCount;
    if (!(successCount > 0))
    {
        m_logger.error(logTag(), QStringLiteral("Failed to generate any entries!"));
        result.code = Error::Code::VocabNoEntries;
    }

    m_logger.verbose(logTag(), QStringLiteral("Handling vocab file entries done!"));
    return result;
}

const VocabGeneration::GenerateVariantsResult VocabGenerator::generateVariants(const VocabFile::Entry &vocabFileEntry)
{
    m_logger.verbose(logTag(), QStringLiteral("Generating variants..."));
    VocabGeneration::GenerateVariantsResult result;
    QSet<VocabFile::Entry> generatedVocabFileEntries;
    QSet<VocabFile::Entry> entriesWithVariants;

    // Separate entries to those with and without variants
    auto sepataredEntries = extractEntriesWithVariants({vocabFileEntry});
    generatedVocabFileEntries.unite(sepataredEntries.first);
    entriesWithVariants = sepataredEntries.second;
    
    m_logger.verbose(logTag(), QStringLiteral("Handling entries with variants..."));
    while(!entriesWithVariants.isEmpty())
    {
        QSet<VocabFile::Entry> newEntries;
        QList<VocabGeneration::BracketType> handleBracketOrder = {VocabGeneration::BracketType::Curly,
                                                                  VocabGeneration::BracketType::Parenthesis,
                                                                  VocabGeneration::BracketType::Square};
        for (const auto &entry : std::as_const(entriesWithVariants))
        {
            m_logger.verbose(logTag(), QStringLiteral("Generating variants for entry '%1 = %2'...").arg(entry.first, entry.second));
            for (const auto &bracketType : handleBracketOrder)
            {
                VocabGeneration::ExtractBracketPartsResult bracketPartsResult = extractInnermostBracketParts(entry, bracketType);
                if (bracketPartsResult.code != Error::Code::Success)
                {
                    m_logger.error(logTag(), QStringLiteral("Failed to extract brackets! ErrorCode %1").arg(Error::codeToQString(bracketPartsResult.code)));
                    result.code = bracketPartsResult.code;
                    return result;
                }

                VocabGeneration::HandleBracketsResult handleBracketResult;
                switch (bracketType)
                {
                case VocabGeneration::BracketType::Curly:
                    handleBracketResult = handleCurlyBraces(bracketPartsResult.parts);
                    break;

                case VocabGeneration::BracketType::Parenthesis:
                    handleBracketResult = handleParenthesis(bracketPartsResult.parts);
                    break;

                case VocabGeneration::BracketType::Square:
                    handleBracketResult = handleSquareBrackets(bracketPartsResult.parts);
                    break;

                default:
                    result.code = Error::Code::UnsupportedBracket;
                    m_logger.error(logTag(), QStringLiteral("Unsupported bracket type!"));
                    return result;
                    break;
                }

                if (handleBracketResult.code != Error::Code::Success)
                {
                    m_logger.error(logTag(), QStringLiteral("Failed to handle brackets! ErrorCode %1").arg(Error::codeToQString(handleBracketResult.code)));
                    result.code = handleBracketResult.code;
                    return result;
                }

                if (!handleBracketResult.entries.isEmpty())
                {
                    // Parse entry only one bracket at a time
                    m_logger.verbose(logTag(), QStringLiteral("Adding newly generated variants..."));
                    newEntries.unite(handleBracketResult.entries);
                    break;
                }
            }
        }

        // Extract entries with variants
        sepataredEntries = extractEntriesWithVariants(newEntries);
        generatedVocabFileEntries.unite(sepataredEntries.first);
        entriesWithVariants = sepataredEntries.second;
    }
    m_logger.verbose(logTag(), QStringLiteral("Handling entries with variants done!"));
    
    m_logger.verbose(logTag(), QStringLiteral("Adding new vocab entries..."));
    for (const auto &newVocabFileEntry : generatedVocabFileEntries)
    {
        result.entries[newVocabFileEntry.first].insert(newVocabFileEntry.second);
    }
    m_logger.verbose(logTag(), QStringLiteral("Adding new vocab entries done!"));

    m_logger.verbose(logTag(), QStringLiteral("Generating variants done!"));
    return result;
}

QPair<QSet<VocabFile::Entry>, QSet<VocabFile::Entry>> VocabGenerator::extractEntriesWithVariants(const QSet<VocabFile::Entry> &entries)
{
    const QStringList variantIndicators = {QStringLiteral("("),
                                           QStringLiteral(")"),
                                           QStringLiteral("["),
                                           QStringLiteral("]"),
                                           QStringLiteral("{"),
                                           QStringLiteral("}")};

    QSet<VocabFile::Entry> entriesWithoutVariants;
    QSet<VocabFile::Entry> entriesWithVariants;
    for (const auto &entry : entries)
    {
        bool hasVariant = false;
        for (const auto &indicator : variantIndicators)
        {
            if (entry.first.contains(indicator) || entry.second.contains(indicator))
            {
                entriesWithVariants.insert(entry);
                hasVariant = true;
                break;
            }
        }

        if (!hasVariant)
        {
            entriesWithoutVariants.insert(entry);
        }
    }

    return {entriesWithoutVariants, entriesWithVariants};
}

const VocabGeneration::ExtractBracketPartsResult VocabGenerator::extractInnermostBracketParts(const VocabFile::Entry &entry, const VocabGeneration::BracketType &type)
{
    VocabGeneration::ExtractBracketPartsResult result;

    // Extract brackets from key
    VocabGeneration::ExtractBracketPartResult keyResult = extractInnermostBracketPart(entry.first, type);
    if (keyResult.code != Error::Code::Success)
    {
        result.code = keyResult.code;
        return result;
    }

    // Extract brackets from value
    VocabGeneration::ExtractBracketPartResult valueResult = extractInnermostBracketPart(entry.second, type);
    if (valueResult.code != Error::Code::Success)
    {
        result.code = valueResult.code;
        return result;
    }

    result.parts = {keyResult.parts, valueResult.parts};
    return result;
}

const VocabGeneration::ExtractBracketPartResult VocabGenerator::extractInnermostBracketPart(const QString &text, const VocabGeneration::BracketType &type)
{
    VocabGeneration::ExtractBracketPartResult result;

    QChar openingBracket;
    QChar closingBracket;
    switch (type)
    {
    case VocabGeneration::BracketType::Parenthesis:
        openingBracket = '(';
        closingBracket = ')';
        break;

    case VocabGeneration::BracketType::Square:
        openingBracket = '[';
        closingBracket = ']';
        break;

    case VocabGeneration::BracketType::Curly:
        openingBracket = '{';
        closingBracket = '}';
        break;

    default:
        result.code = Error::Code::UnsupportedBracket;
        return result;
        break;
    }

    struct BracketPosition
    {
        QChar bracket;
        int position;
    };
    QStack<BracketPosition> stack;

    int innermostStart = -1;
    int innermostEnd = -1;

    // Go through text and extract positions of innermost brackets
    for (int i = 0; i < text.size(); i++)
    {
        const QChar ch = text[i];

        if (ch == '(' || ch == '[' || ch == '{')
        {
            stack.push({ch, i});
        }
        else if (ch == ')' || ch == ']' || ch == '}')
        {
            if (stack.isEmpty())
            {
                result.code = Error::Code::MismatchingBrackets;
                return result;
            }

            // Check if closing bracket matches with opening bracket
            const BracketPosition opening = stack.pop();
            const bool matches = (opening.bracket == '(' && ch == ')') ||
                                 (opening.bracket == '[' && ch == ']') ||
                                 (opening.bracket == '{' && ch == '}');
            if (!matches)
            {
                result.code = Error::Code::MismatchingBrackets;
                return result;
            }

            // Keep first completed pair of requested type
            if (innermostStart == -1 && opening.bracket == openingBracket)
            {
                innermostStart = opening.position;
                innermostEnd = i;
            }
        }
    }

    // All opening brackets must have a matching closing bracket.
    if (!stack.isEmpty())
    {
        result.code = Error::Code::MismatchingBrackets;
        return result;
    }

    // Bracket type not found in text
    if (innermostStart == -1)
    {
        result.parts.before = text.trimmed();
        return result;
    }

    // Construct bracket parts
    result.parts.before = text.first(innermostStart).trimmed();
    result.parts.inside = text.mid(innermostStart + 1, innermostEnd - (innermostStart + 1)).trimmed();
    result.parts.after = text.mid(innermostEnd + 1).trimmed();

    return result;
}

const QString VocabGenerator::toString(VocabGeneration::BracketParts parts, bool inside)
{
    QString result;
    bool spaceAfterFirstPart = true;
    bool spaceBeforeLastPart = true;

    if (parts.before.endsWith('_'))
    {
        spaceAfterFirstPart = false;
        parts.before.chop(1);
    }

    if (parts.after.startsWith('_'))
    {
        spaceBeforeLastPart = false;
        parts.after.remove(0, 1);
    }

    result.append(parts.before);
    result.append(spaceAfterFirstPart ? " " : "");
    result.append(inside ? parts.inside : "");
    result.append(spaceBeforeLastPart ? " " : "");
    result.append(parts.after);
    result = result.trimmed();

    return result;
}

const VocabGeneration::HandleBracketsResult VocabGenerator::handleCurlyBraces(const QPair<VocabGeneration::BracketParts, VocabGeneration::BracketParts> &parts)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling variants with curly braces..."));
    VocabGeneration::HandleBracketsResult result;

    VocabGeneration::BracketParts keyParts = parts.first;
    VocabGeneration::BracketParts valueParts = parts.second;

    if (keyParts.inside.isEmpty() && valueParts.inside.isEmpty())
    {
        // Nothing to generate
        m_logger.verbose(logTag(), QStringLiteral("Nothing to generate. Returning..."));
        return result;
    }

    // Generate cartesian product of entries
    result.entries.unite({{toString(keyParts), toString(valueParts)},
                          {toString(keyParts), toString(valueParts, false)},
                          {toString(keyParts, false), toString(valueParts)},
                          {toString(keyParts, false), toString(valueParts, false)}});

    m_logger.verbose(logTag(), QStringLiteral("Handling variants with curly braces done!"));
    return result;
}

const VocabGeneration::HandleBracketsResult VocabGenerator::handleParenthesis(const QPair<VocabGeneration::BracketParts, VocabGeneration::BracketParts> &parts)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling variants with parenthesis..."));
    VocabGeneration::HandleBracketsResult result;

    VocabGeneration::BracketParts keyParts = parts.first;
    VocabGeneration::BracketParts valueParts = parts.second;

    if (keyParts.inside.isEmpty() && valueParts.inside.isEmpty())
    {
        // Nothing to generate
        m_logger.verbose(logTag(), QStringLiteral("Nothing to generate. Returning..."));
        return result;
    }

    // Generate 1:1 mapping
    if (keyParts.inside.isEmpty())
    {
        result.entries.unite({{toString(keyParts), toString(valueParts)},
                              {toString(keyParts), toString(valueParts, false)}});
    }
    else if (valueParts.inside.isEmpty())
    {
        result.entries.unite({{toString(keyParts), toString(valueParts)},
                              {toString(keyParts, false), toString(valueParts)}});
    }
    else
    {
        result.entries.unite({{toString(keyParts), toString(valueParts)},
                              {toString(keyParts, false), toString(valueParts, false)}});
    }

    m_logger.verbose(logTag(), QStringLiteral("Handling variants with parenthesis done!"));
    return result;
}

const VocabGeneration::HandleBracketsResult VocabGenerator::handleSquareBrackets(const QPair<VocabGeneration::BracketParts, VocabGeneration::BracketParts> &parts)
{
    m_logger.verbose(logTag(), QStringLiteral("Handling variants with square brackets..."));

    VocabGeneration::HandleBracketsResult result;

    VocabGeneration::BracketParts keyParts = parts.first;
    VocabGeneration::BracketParts valueParts = parts.second;

    if (keyParts.inside.isEmpty() && valueParts.inside.isEmpty())
    {
        // Nothing to generate
        m_logger.verbose(logTag(), QStringLiteral("Nothing to generate. Returning..."));
        return result;
    }

    // Construct options indicated by '/'
    if (keyParts.inside.isEmpty())
    {
        QStringList valueOptions = valueParts.inside.split('/');
        for (auto const &option : valueOptions)
        {
            VocabGeneration::BracketParts valueOptionParts;
            valueOptionParts.before = valueParts.before;
            valueOptionParts.inside = option.trimmed();
            valueOptionParts.after = valueParts.after;

            result.entries.unite({{toString(keyParts), toString(valueOptionParts)}});
        }
    }
    else if (valueParts.inside.isEmpty())
    {
        QStringList keyOptions = keyParts.inside.split('/');
        for (auto const &option : keyOptions)
        {
            VocabGeneration::BracketParts keyOptionParts;
            keyOptionParts.before = keyParts.before;
            keyOptionParts.inside = option.trimmed();
            keyOptionParts.after = keyParts.after;

            result.entries.unite({{toString(keyOptionParts), toString(valueParts)}});
        }
    }
    else
    {
        QStringList keyOptions = keyParts.inside.split('/');
        QStringList valueOptions = valueParts.inside.split('/');

        if(keyOptions.size() != valueOptions.size())
        {
            result.code = Error::Code::MismatchingOptions;
            return result;
        }
        int optionsCount = keyOptions.count();
        for (int i = 0; i < optionsCount; i++)
        {
            VocabGeneration::BracketParts keyOptionParts;
            keyOptionParts.before = keyParts.before;
            keyOptionParts.inside = keyOptions.at(i).trimmed();
            keyOptionParts.after = keyParts.after;

            VocabGeneration::BracketParts valueOptionParts;
            valueOptionParts.before = valueParts.before;
            valueOptionParts.inside = valueOptions.at(i).trimmed();
            valueOptionParts.after = valueParts.after;

            result.entries.unite({{toString(keyOptionParts), toString(valueOptionParts)}});
        }
    }

    m_logger.verbose(logTag(), QStringLiteral("Handling variants with square brackets done!"));
    return result;
}

void VocabGenerator::uniteEntries(Vocab::Entries &entries, const Vocab::Entries &newEntries)
{
    for (auto it = newEntries.cbegin(); it != newEntries.cend(); ++it)
    {
        // Insert new key if needed and combine new values with existing values
        // it.key() is entry key
        // it.value() is list of values for that key
        entries[it.key()].unite(it.value());
    }
}

void VocabGenerator::uniteGroups(Vocab::GroupEntries &groupEntries, const Vocab::GroupEntries &newGroupEntries)
{
    for (auto it = newGroupEntries.cbegin(); it != newGroupEntries.cend(); ++it)
    {
        // Insert new group if needed and combine new group values with existing values
        // it.key() is title of group
        // it.value() is group entries of that group
        uniteEntries(groupEntries[it.key()], it.value());
    }
}
