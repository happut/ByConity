/*
 * Copyright (2022) Bytedance Ltd. and/or its affiliates
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Interpreters/ProcessorsProfileLog.h>

#include <Common/ClickHouseRevision.h>
#include <DataTypes/DataTypeDate.h>
#include <DataTypes/DataTypeDateTime.h>
#include <DataTypes/DataTypeDateTime64.h>
#include <DataTypes/DataTypeLowCardinality.h>
#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <DataTypes/DataTypeNullable.h>
#include <DataTypes/DataTypeArray.h>
#include <common/logger_useful.h>

#include <array>

namespace DB
{

NamesAndTypesList ProcessorProfileLogElement::getNamesAndTypes()
{
    return
    {
        {"event_date", std::make_shared<DataTypeDate>()},       
        {"event_time", std::make_shared<DataTypeDateTime>()},
        {"event_time_microseconds", std::make_shared<DataTypeDateTime64>(6)},

        {"id", std::make_shared<DataTypeUInt64>()},
        {"parent_ids", std::make_shared<DataTypeArray>(std::make_shared<DataTypeUInt64>())},
        {"plan_step", std::make_shared<DataTypeUInt64>()},
        {"plan_group", std::make_shared<DataTypeUInt64>()},
        {"query_id", std::make_shared<DataTypeString>()},
        {"name", std::make_shared<DataTypeLowCardinality>(std::make_shared<DataTypeString>())},
        {"elapsed_us", std::make_shared<DataTypeUInt64>()},
        {"input_wait_elapsed_us", std::make_shared<DataTypeUInt64>()},
        {"output_wait_elapsed_us", std::make_shared<DataTypeUInt64>()},
        {"input_rows", std::make_shared<DataTypeUInt64>()},
        {"input_bytes", std::make_shared<DataTypeUInt64>()},
        {"output_rows", std::make_shared<DataTypeUInt64>()},
        {"output_bytes", std::make_shared<DataTypeUInt64>()},
    };
}

void ProcessorProfileLogElement::appendToBlock(MutableColumns & columns) const
{
    size_t i = 0;

    columns[i++]->insert(DateLUT::instance().toDayNum(event_time).toUnderType());
    columns[i++]->insert(event_time);
    columns[i++]->insert(event_time_microseconds);

    columns[i++]->insert(id);
    {
        Array parent_ids_array;
        parent_ids_array.reserve(parent_ids.size());
        for (const UInt64 parent : parent_ids)
            parent_ids_array.emplace_back(parent);
        columns[i++]->insert(parent_ids_array);
    }
    columns[i++]->insert(plan_step);
    columns[i++]->insert(plan_group);
    columns[i++]->insertData(query_id.data(), query_id.size());
    columns[i++]->insertData(processor_name.data(), processor_name.size());
    columns[i++]->insert(elapsed_us);
    columns[i++]->insert(input_wait_elapsed_us);
    columns[i++]->insert(output_wait_elapsed_us);
    columns[i++]->insert(input_rows);
    columns[i++]->insert(input_bytes);
    columns[i++]->insert(output_rows);
    columns[i++]->insert(output_bytes);
}

ProcessorsProfileLog::ProcessorsProfileLog(ContextPtr context_, const String & database_name_,
        const String & table_name_, const String & storage_def_,
        size_t flush_interval_milliseconds_)
  : SystemLog<ProcessorProfileLogElement>(context_, database_name_, table_name_,
        storage_def_, flush_interval_milliseconds_)
{
}

}
