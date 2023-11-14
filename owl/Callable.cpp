// ======================================================================== //
// Copyright 2019 Ingo Wald                                                 //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "Callable.h"
#include "Context.h"

namespace owl {

    // ------------------------------------------------------------------
    // CallableType::DeviceData
    // ------------------------------------------------------------------

    /*! constructor, only pass-through to parent class */
    CallableType::DeviceData::DeviceData(const DeviceContext::SP& device)
        : RegisteredObject::DeviceData(device)
    {}

    // ------------------------------------------------------------------
    // CallableType
    // ------------------------------------------------------------------

    CallableType::CallableType(Context* const context,
        Module::SP module,
        const std::string& progName,
        size_t varStructSize,
        const std::vector<OWLVarDecl>& varDecls, 
        bool direct_callable)
        : SBTObjectType(context, context->callableTypes, varStructSize, varDecls),
        module(module),
        progName(progName),
        annotatedProgName(
            (direct_callable?"__direct_callable__":"__continuation_callable__") 
            + 
        progName),
        is_direct_callable{direct_callable}
    {}

    /*! pretty-printer, for printf-debugging */
    std::string CallableType::toString() const
    {
        return "CallableType";
    }

    /*! creates the device-specific data for this group */
    RegisteredObject::DeviceData::SP
        CallableType::createOn(const DeviceContext::SP& device)
    {
        return std::make_shared<DeviceData>(device);
    }

    // ------------------------------------------------------------------
    // Callable
    // ------------------------------------------------------------------

    Callable::Callable(Context* const context,
        CallableType::SP type)
        : SBTObject(context, context->callables, type)
    {}

    /*! pretty-printer, for printf-debugging */
    std::string Callable::toString() const
    {
        return "Callable";
    }

    /*! write the given SBT record, using the given device's
    corresponding device-side data represenataion */
    void Callable::writeSBTRecord(uint8_t* const sbtRecord,
        const DeviceContext::SP& device)
    {
        auto& dd = type->getDD(device);

        // first, compute pointer to record:
        uint8_t* const sbtRecordHeader = sbtRecord;
        uint8_t* const sbtRecordData = sbtRecord + OPTIX_SBT_RECORD_HEADER_SIZE;

        // ------------------------------------------------------------------
        // pack record header with the corresponding hit group:
        // ------------------------------------------------------------------
        OPTIX_CALL(SbtRecordPackHeader(dd.pg, sbtRecordHeader));

        // ------------------------------------------------------------------
        // then, write the data for that record
        // ------------------------------------------------------------------
        writeVariables(sbtRecordData, device);
    }

} // ::owl

