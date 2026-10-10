#include "vocabgenerator.h"

namespace
{
    const QString &logTag()
    {
        static const QString tag = QStringLiteral("VocabFileHandler");
        return tag;
    }
}

VocabGenerator::VocabGenerator(Logger &logger)
    : m_logger(logger)
{
    m_logger.verbose(logTag(), QStringLiteral("Constructing vocab generator..."));
    m_logger.verbose(logTag(), QStringLiteral("Constructing vocab generator done!"));
}

const GenerateVocabDataResult VocabGenerator::generate(const VocabFile::Data &vocabFileData)
{
    GenerateVocabDataResult result;
    result.data.title = vocabFileData.info.vocabTitle;

    // Generate entries without group
    GenerateVocabEntriesResult generateEntriesResult = handleVocabFileEntries(vocabFileData.entries);
    if (generateEntriesResult.code == ErrorCode::Success)
    {
        // unite only successfully generated entries
        uniteEntries(result.data.entries, generateEntriesResult.entries);
    }

    if (generateEntriesResult.failedEntryCount > 0)
    {

    }
    result.failedEntryCount += generateEntriesResult.failedEntryCount;

    // Generate entries under groups
    for (auto const &group : vocabFileData.groups)
    {
        GenerateVocabEntriesResult generateGroupEntriesResult = handleVocabFileEntries(group.entries);
        if (generateGroupEntriesResult.code != ErrorCode::Success)
        {
            result.failedGroupCount++;
            continue;
        }

        if (generateGroupEntriesResult.failedEntryCount > 0)
        {
            result.partiallySucceededGroupCount++;
        }
        result.failedEntryCount += generateGroupEntriesResult.failedEntryCount;

        Vocab::GroupEntries newGroupEntries;
        newGroupEntries.insert(group.title, generateGroupEntriesResult.entries);

        uniteGroups(result.data.groupEntries, newGroupEntries);
    }

    return result;
}

const GenerateVocabEntriesResult VocabGenerator::handleVocabFileEntries(const QList<VocabFile::Entry> &entries)
{
    GenerateVocabEntriesResult result;
    for (const auto &entry : entries)
    {
        GenerateVocabEntryVariantsResult generateVariantsResult = generateVariants(entry);
        if (generateVariantsResult.code != ErrorCode::Success)
        {
            result.failedEntryCount++;
            continue;
        }
        uniteEntries(result.entries, generateVariantsResult.entries);
    }

    int successCount = entries.count() - result.failedEntryCount;
    if (!(successCount > 0))
    {
        result.code = ErrorCode::VocabNoEntries;
    }

    return result;
}

const GenerateVocabEntryVariantsResult VocabGenerator::generateVariants(const VocabFile::Entry &vocabFileEntry)
{
    GenerateVocabEntryVariantsResult result;
    QList<VocabFile::Entry> generatedVocabFileEntries;
    QList<VocabFile::Entry> entriesWithVariants;

    auto sepataredEntries = extractEntriesWithVariants({vocabFileEntry});
    generatedVocabFileEntries.append(sepataredEntries.first);
    entriesWithVariants = sepataredEntries.second;
    
    while(!entriesWithVariants.isEmpty())
    {
        QList<VocabFile::Entry> newEntries;
        for (const auto &entry : entriesWithVariants)
        {
            ExtractBracketPartsResult bracketPartsResult;
            HandleBracketsResult handleBracketResult;

            // Handle curly braces
            bracketPartsResult = extractBracketParts(entry, BracketType::Curly);
            if (bracketPartsResult.code != ErrorCode::Success)
            {
                result.code = bracketPartsResult.code;
                return result;
            }

            handleBracketResult = handleCurlyBraces(bracketPartsResult.parts);
            if (handleBracketResult.code != ErrorCode::Success)
            {
                result.code = handleBracketResult.code;
                return result;
            }

            if (!handleBracketResult.entries.isEmpty())
            {
                // Parse entry only one bracket at a time
                newEntries.append(handleBracketResult.entries);
                continue;
            }

            // Handle parenthesis
            bracketPartsResult = extractBracketParts(entry, BracketType::Parenthesis);
            if (bracketPartsResult.code != ErrorCode::Success)
            {
                result.code = bracketPartsResult.code;
                return result;
            }

            handleBracketResult = handleParenthesis(bracketPartsResult.parts);
            if (handleBracketResult.code != ErrorCode::Success)
            {
                result.code = handleBracketResult.code;
                return result;
            }

            if (!handleBracketResult.entries.isEmpty())
            {
                // Parse entry only one bracket at a time
                newEntries.append(handleBracketResult.entries);
                continue;
            }

            // Handle square brackets
            bracketPartsResult = extractBracketParts(entry, BracketType::Square);
            if (bracketPartsResult.code != ErrorCode::Success)
            {
                result.code = bracketPartsResult.code;
                return result;
            }

            handleBracketResult = handleSquareBrackets(bracketPartsResult.parts);
            if (handleBracketResult.code != ErrorCode::Success)
            {
                result.code = handleBracketResult.code;
                return result;
            }

            if (!handleBracketResult.entries.isEmpty())
            {
                // Parse entry only one bracket at a time
                newEntries.append(handleBracketResult.entries);
                continue;
            }
        }

        // Extract entries with variants
        sepataredEntries = extractEntriesWithVariants(newEntries);
        generatedVocabFileEntries.append(sepataredEntries.first);
        entriesWithVariants = sepataredEntries.second;
    }
    
    for (const auto &newVocabFileEntry : generatedVocabFileEntries)
    {
        result.entries[newVocabFileEntry.first].insert(newVocabFileEntry.second);
    }

    return result;
}

QPair<QList<VocabFile::Entry>, QList<VocabFile::Entry> > VocabGenerator::extractEntriesWithVariants(const QList<VocabFile::Entry> &entries)
{
    const QStringList variantIndicators = {QStringLiteral("("),
                                           QStringLiteral(")"),
                                           QStringLiteral("["),
                                           QStringLiteral("]"),
                                           QStringLiteral("{"),
                                           QStringLiteral("}")};

    QList<VocabFile::Entry> entriesWithoutVariants;
    QList<VocabFile::Entry> entriesWithVariants;
    for (const auto &entry : entries)
    {
        for (const auto &indicator : variantIndicators)
        {
            if (entry.first.contains(indicator) || entry.second.contains(indicator))
            {
                entriesWithVariants.append(entry);
                continue;
            }

            entriesWithoutVariants.append(entry);
        }
    }

    return {entriesWithoutVariants, entriesWithVariants};
}

const ExtractBracketPartsResult VocabGenerator::extractBracketParts(const VocabFile::Entry &entry, const BracketType &type)
{
    ExtractBracketPartsResult result;

    // Extract brackets from key
    ExtractBracketPartResult keyResult = extractBracketPart(entry.first, type);
    if (keyResult.code != ErrorCode::Success)
    {
        result.code = keyResult.code;
        return result;
    }

    // Extract brackets from value
    ExtractBracketPartResult valueResult = extractBracketPart(entry.second, type);
    if (valueResult.code != ErrorCode::Success)
    {
        result.code = valueResult.code;
        return result;
    }

    result.parts = {keyResult.parts, valueResult.parts};
    return result;
}

const ExtractBracketPartResult VocabGenerator::extractBracketPart(const QString &text, const BracketType &type)
{
    ExtractBracketPartResult result;
    QChar openingBracket;
    QChar closingBracket;
    switch (type)
    {
    case BracketType::Parenthesis:
        openingBracket = '(';
        closingBracket = ')';
        break;

    case BracketType::Square:
        openingBracket = '[';
        closingBracket = ']';
        break;

    case BracketType::Curly:
        openingBracket = '{';
        closingBracket = '}';
        break;

    default:
        result.code = ErrorCode::UnsupportedBracket;
        return result;
        break;
    }

    int start = -1;
    int bracketCount = 0;

    for (int i = 0; i < text.size(); i++)
    {
        const QChar ch = text[i];
        if (ch == openingBracket)
        {
            if (bracketCount == 0)
            {
                start = i;
            }

            bracketCount++;
        }
        else if (ch == closingBracket)
        {
            if (bracketCount == 0)
            {
                result.code = ErrorCode::MismatchingBrackets;
                return result;
            }

            bracketCount--;

            if (bracketCount == 0)
            {
                result.parts.before = text.first(start).trimmed();
                result.parts.inside = text.mid(start + 1, i - (start + 1)).trimmed();
                result.parts.after = text.mid(i+1).trimmed();
                return result;
            }
        }
    }

    if (start == -1)
    {
        result.parts.before = text;
    }

    return result;
}

const QString VocabGenerator::toString(const BracketParts &parts, bool inside)
{
    if (!inside)
    {
        return QStringLiteral("%1 %3").arg(parts.before, parts.after);
    }

    return QStringLiteral("%1 %2 %3").arg(parts.before, parts.inside, parts.after);
}

const HandleBracketsResult VocabGenerator::handleCurlyBraces(const QPair<BracketParts, BracketParts> &parts)
{
    HandleBracketsResult result;

    BracketParts keyParts = parts.first;
    BracketParts valueParts = parts.second;

    if (keyParts.inside.isEmpty() && valueParts.inside.isEmpty())
    {
        // Nothing to generate
        return result;
    }

    // Generate cartesian product of entries
    result.entries.append({{toString(keyParts), toString(valueParts)},
                       {toString(keyParts), toString(valueParts, false)},
                       {toString(keyParts, false), toString(valueParts)},
                       {toString(keyParts, false), toString(valueParts, false)}});

    return result;
}

const HandleBracketsResult VocabGenerator::handleParenthesis(const QPair<BracketParts, BracketParts> &parts)
{
    HandleBracketsResult result;

    BracketParts keyParts = parts.first;
    BracketParts valueParts = parts.second;

    if (keyParts.inside.isEmpty() && valueParts.inside.isEmpty())
    {
        // Nothing to generate
        return result;
    }

    // Generate 1:1 mapping
    if (keyParts.inside.isEmpty())
    {
        result.entries.append({{toString(keyParts), toString(valueParts)},
                           {toString(keyParts), toString(valueParts, false)}});
    }
    else if (valueParts.inside.isEmpty())
    {
        result.entries.append({{toString(keyParts), toString(valueParts)},
                           {toString(keyParts, false), toString(valueParts)}});
    }
    else
    {
        result.entries.append({{toString(keyParts), toString(valueParts)},
                           {toString(keyParts, false), toString(valueParts, false)}});
    }

    return result;
}

const HandleBracketsResult VocabGenerator::handleSquareBrackets(const QPair<BracketParts, BracketParts> &parts)
{
    HandleBracketsResult result;

    BracketParts keyParts = parts.first;
    BracketParts valueParts = parts.second;

    if (keyParts.inside.isEmpty() && valueParts.inside.isEmpty())
    {
        // Nothing to generate
        return result;
    }

    // Construct options indicated by '/'
    if (keyParts.inside.isEmpty())
    {
        QStringList valueOptions = valueParts.inside.split('/');
        for (auto const &option : valueOptions)
        {
            BracketParts valueOptionParts;
            valueOptionParts.before = valueParts.before;
            valueOptionParts.inside = option.trimmed();
            valueOptionParts.after = valueParts.after;

            result.entries.append({{toString(keyParts), toString(valueOptionParts)}});
        }
    }
    else if (valueParts.inside.isEmpty())
    {
        QStringList keyOptions = keyParts.inside.split('/');
        for (auto const &option : keyOptions)
        {
            BracketParts keyOptionParts;
            keyOptionParts.before = keyParts.before;
            keyOptionParts.inside = option.trimmed();
            keyOptionParts.after = keyParts.after;

            result.entries.append({{toString(keyOptionParts), toString(valueParts)}});
        }
    }
    else
    {
        QStringList keyOptions = keyParts.inside.split('/');
        QStringList valueOptions = valueParts.inside.split('/');

        if(keyOptions.size() != valueOptions.size())
        {
            result.code = ErrorCode::MismatchingOptions;
            return result;
        }
        int optionsCount = keyOptions.count();
        for (int i = 0; i < optionsCount; i++)
        {
            BracketParts keyOptionParts;
            keyOptionParts.before = keyParts.before;
            keyOptionParts.inside = keyOptions.at(i).trimmed();
            keyOptionParts.after = keyParts.after;

            BracketParts valueOptionParts;
            valueOptionParts.before = valueParts.before;
            valueOptionParts.inside = valueOptions.at(i).trimmed();
            valueOptionParts.after = valueParts.after;

            result.entries.append({{toString(keyOptionParts), toString(valueOptionParts)}});
        }
    }

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
