////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Learning Library (ELL)
//  File:     Map.cpp (model)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Map.h"
#include "ModelTransformer.h"
#include "OptimizeModelTransformation.h"
#include "OutputNode.h"
#include "RefineTransformation.h"

#include <utilities/include/Exception.h>
#include <utilities/include/StlVectorUtil.h>

#include <algorithm>
#include <iomanip>
#include <unordered_set>

namespace ell
{
namespace model
{
    namespace
    {
        //
        // Relevant archive format versions
        //
        constexpr utilities::ArchiveVersion noMetadataArchiveVersion = { utilities::ArchiveVersionNumbers::v2 };
        constexpr utilities::ArchiveVersion metadataArchiveVersion = { utilities::ArchiveVersionNumbers::v3_model_metadata };
    } // namespace

    Map::Map(const Model& model, const std::vector<std::pair<std::string, InputNodeBase*>>& inputs, const std::vector<std::pair<std::string, PortElementsBase>>& outputs)
    {
        TransformContext context;
        ModelTransformer transformer;
        _model = transformer.CopyModel(model, context);

        for (const auto& input : inputs)
        {
            AddInput(input.first, transformer.GetCorrespondingInputNode(input.second));
        }

        for (const auto& output : outputs)
        {
            if (!output.second.IsFullPortOutput())
            {
                throw utilities::LogicException(utilities::LogicExceptionErrors::notImplemented, "Map constructor requires full output ports (IsFullPortOutput()==true)");
            }
            AddOutput(output.first, transformer.GetCorrespondingOutputs(output.second));
        }

        Prune();
        _model.Verify();
    }

    Map::Map(Model&& model, const std::vector<std::pair<std::string, InputNodeBase*>>& inputs, const std::vector<std::pair<std::string, PortElementsBase>>& outputs) :
        _model(std::move(model))
    {
        for (const auto& input : inputs)
        {
            AddInput(input.first, input.second);
        }

        for (const auto& output : outputs)
        {
            if (!output.second.IsFullPortOutput())
            {
                throw utilities::LogicException(utilities::LogicExceptionErrors::notImplemented, "Map constructor requires full output ports (IsFullPortOutput()==true)");
            }
            AddOutput(output.first, output.second);
        }

        Prune();
        _model.Verify();
    }

    Map::Map(const Map& other)
    {
        TransformContext context;
        ModelTransformer transformer;
        _model = transformer.CopyModel(other._model, context);
        for (const auto& input : other._inputNodeMap)
        {
            AddInput(input.first, transformer.GetCorrespondingInputNode(input.second));
        }

        for (const auto& output : other._outputElementsMap)
        {
            AddOutput(output.first, transformer.GetCorrespondingOutputs(output.second));
        }

        _model.Verify();
    }

    Map& Map::operator=(Map other)
    {
        swap(*this, other);
        return *this;
    }

    Submodel Map::GetSubmodel()
    {
        return { _model };
    }

    void Map::SetNodeInput(InputNode<bool>* node, const std::vector<bool>& inputValues)
    {
        node->SetInput(inputValues);
    }

    void Map::SetNodeInput(InputNode<int>* node, const std::vector<int>& inputValues)
    {
        node->SetInput(inputValues);
    }

    void Map::SetNodeInput(InputNode<int64_t>* node, const std::vector<int64_t>& inputValues)
    {
        node->SetInput(inputValues);
    }

    void Map::SetNodeInput(InputNode<float>* node, const std::vector<float>& inputValues)
    {
        node->SetInput(inputValues);
    }

    void Map::SetNodeInput(InputNode<double>* node, const std::vector<double>& inputValues)
    {
        node->SetInput(inputValues);
    }

    std::vector<bool> Map::ComputeBoolOutput(const PortElementsBase& outputs)
    {
        return _model.ComputeOutput<bool>(outputs);
    }

    std::vector<int> Map::ComputeIntOutput(const PortElementsBase& outputs)
    {
        return _model.ComputeOutput<int>(outputs);
    }

    std::vector<int64_t> Map::ComputeInt64Output(const PortElementsBase& outputs)
    {
        return _model.ComputeOutput<int64_t>(outputs);
    }

    std::vector<float> Map::ComputeFloatOutput(const PortElementsBase& outputs)
    {
        return _model.ComputeOutput<float>(outputs);
    }

    std::vector<double> Map::ComputeDoubleOutput(const PortElementsBase& outputs)
    {
        return _model.ComputeOutput<double>(outputs);
    }

    template <>
    std::vector<bool> Map::ComputeOutput<bool>(const PortElementsBase& elements)
    {
        value::ContextGuard<> guard(_computeContext);

        return ComputeBoolOutput(elements);
    }

    template <>
    std::vector<int> Map::ComputeOutput<int>(const PortElementsBase& elements)
    {
        value::ContextGuard<> guard(_computeContext);

        return ComputeIntOutput(elements);
    }

    template <>
    std::vector<int64_t> Map::ComputeOutput<int64_t>(const PortElementsBase& elements)
    {
        value::ContextGuard<> guard(_computeContext);

        return ComputeInt64Output(elements);
    }

    template <>
    std::vector<float> Map::ComputeOutput<float>(const PortElementsBase& elements)
    {
        value::ContextGuard<> guard(_computeContext);

        return ComputeFloatOutput(elements);
    }

    template <>
    std::vector<double> Map::ComputeOutput<double>(const PortElementsBase& elements)
    {
        value::ContextGuard<> guard(_computeContext);

        return ComputeDoubleOutput(elements);
    }

    void Map::Reset()
    {
        _model.Reset();
    }

    void Map::AddInput(const std::string& inputName, InputNodeBase* inputNode)
    {
        _inputNodes.push_back(inputNode);
        _inputNames.push_back(inputName);
        _inputNodeMap.insert({ inputName, inputNode });
    }

    void Map::RemoveInputs()
    {
        _inputNodes.clear();
        _inputNames.clear();
        _inputNodeMap.clear();
    }

    void Map::AddOutput(const std::string& outputName, PortElementsBase outputElements)
    {
        // Add concat/splice nodes to ensure output is a single port
        const auto& newOutputPort = _model.SimplifyOutputs(outputElements);
        PortElementsBase newOutputElements{ newOutputPort };
        _outputElements.push_back({ newOutputPort });
        _outputNames.push_back(outputName);
        _outputElementsMap.insert({ outputName, newOutputElements });
    }

    void swap(Map& a, Map& b)
    {
        using std::swap;
        swap(a._model, b._model);
        swap(a._inputNodes, b._inputNodes);
        swap(a._inputNames, b._inputNames);
        swap(a._inputNodeMap, b._inputNodeMap);
        swap(a._outputElements, b._outputElements);
        swap(a._outputNames, b._outputNames);
        swap(a._outputElementsMap, b._outputElementsMap);
        swap(a._computeContext, b._computeContext);
    }

    std::vector<const Node*> Map::GetAllOutputNodes() const
    {
        // gather output nodes
        std::unordered_set<const Node*> outputNodes;
        for (const auto& output : GetOutputs())
        {
            for (const auto& range : output.GetRanges())
            {
                outputNodes.insert(range.ReferencedPort()->GetNode());
            }
        }

        return { outputNodes.begin(), outputNodes.end() };
    }

    std::vector<const Node*> Map::GetDebugSinkNodes() const
    {
        std::unordered_set<const Node*> sinkNodes;
        for (const Node* node : GetMatchingNodesByType("DebugSinkNode"))
        {
            auto parents = node->GetParentNodes();
            for (auto ptr = parents.begin(), end = parents.end(); ptr != end; ptr++)
            {
                const Node* parent = *ptr;
                auto dependents = parent->GetDependentNodes();
                for (auto ptr2 = dependents.begin(), end2 = dependents.end(); ptr2 != end2; ptr2++)
                {
                    const Node* dep = *ptr2;
                    if (dep != node)
                    {
                        // then we want to keep this DebugSinkNode, it should not get pruned.
                        sinkNodes.insert(node);
                        break;
                    }
                }
            }
        };
        return { sinkNodes.begin(), sinkNodes.end() };
    }

    std::vector<const Node*> Map::GetMatchingNodesByType(const std::string name) const
    {
        // gather nodes whose runtime type name contains the given substring.
        std::unordered_set<const Node*> result;
        _model.Visit([&](const Node& node) {
            if (node.GetRuntimeTypeName().find(name) != std::string::npos)
            {
                // then we want to keep this node it should not get pruned.
                result.insert(&node);
            }
        });
        return { result.begin(), result.end() };
    }

    void Map::FixTransformedIO(ModelTransformer& transformer)
    {
        for (auto& inputNode : _inputNodes)
        {
            auto refinedInput = transformer.GetCorrespondingInputNode(inputNode);
            inputNode = refinedInput;
        }

        for (auto& inputNode : _inputNodeMap)
        {
            auto input = inputNode.second;
            auto refinedInput = transformer.GetCorrespondingInputNode(input);
            inputNode.second = refinedInput;
        }

        for (auto& outputElements : _outputElements)
        {
            const auto& refinedOutput = transformer.GetCorrespondingOutputs(outputElements);
            outputElements = { refinedOutput };
        }

        for (auto& outputElements : _outputElementsMap)
        {
            auto output = outputElements.second;
            const auto& refinedOutput = transformer.GetCorrespondingOutputs(output);
            outputElements.second = { refinedOutput };
        }
    }

    void Map::Prune()
    {
        auto outputNodes = GetAllOutputNodes();
        auto debugSinkNodes = GetDebugSinkNodes();
        outputNodes.insert(outputNodes.end(), debugSinkNodes.begin(), debugSinkNodes.end());

        // add any additional sink nodes not mentioned in the "outputs" list.
        auto sinkNodes = GetSinkNodes();
        std::unordered_set<const Node*> existing(outputNodes.begin(), outputNodes.end());
        for (auto node : sinkNodes)
        {
            if (existing.find(node) == existing.end())
            {
                outputNodes.push_back(node);
            }
        }

        std::vector<const OutputPortBase*> outputPorts;
        for (auto node : outputNodes)
        {
            for (auto port : node->GetOutputPorts())
            {
                outputPorts.push_back(port);
            }
        }

        TransformContext context;
        ModelTransformer transformer;
        Submodel submodel(_model, {}, outputPorts);
        auto minimalModel = transformer.CopySubmodel(submodel, context);
        FixTransformedIO(transformer);
        _model = minimalModel.GetModel().ShallowCopy();
        _model.Verify();
    }

    size_t Map::NumInputs() const
    {
        return _inputNodes.size();
    }

    size_t Map::GetInputSize(size_t index) const
    {
        // If we have SourceNodes then these override the InputNodes in terms of our compiled API.
        auto sourceNodes = _model.GetNodesByType<SourceNodeBase>();
        if (index < sourceNodes.size())
        {
            return sourceNodes[index]->GetShape().NumElements();
        }
        return GetInputShape(index).NumElements();
    }

    MemoryShape Map::GetInputShape(size_t index) const
    {
        // If we have SourceNodes then these override the InputNodes in terms of our compiled API.
        auto sourceNodes = _model.GetNodesByType<SourceNodeBase>();
        if (index < sourceNodes.size())
        {
            return sourceNodes[index]->GetShape();
        }
        // no source nodes, fallback to first input node's shape
        return GetInput(index)->GetShape();
    }

    std::vector<const InputNodeBase*> Map::GetInputNodes() const
    {
        // no source nodes, fallback to regular input nodes
        return std::vector<const InputNodeBase*>(_inputNodes.begin(), _inputNodes.end());
    }

    std::vector<const SourceNodeBase*> Map::GetSourceNodes() const
    {
        return _model.GetNodesByType<SourceNodeBase>();
    }

    size_t Map::NumOutputs() const
    {
        return _outputElements.size();
    }

    size_t Map::GetOutputSize(size_t index) const
    {
        return GetOutputShape(index).NumElements();
    }

    size_t Map::GetNumSinkNodes() const
    {
        auto nodes = GetSinkNodes();
        return nodes.size();
    }

    size_t Map::GetSinkOutputSize(size_t index) const
    {
        return GetSinkOutputShape(index).NumElements();
    }

    std::vector<const OutputNodeBase*> Map::GetOutputNodes() const
    {
        std::vector<const OutputNodeBase*> result;
        auto allOutputs = GetAllOutputNodes();
        for (auto iter = allOutputs.begin(), end = allOutputs.end(); iter != end; iter++)
        {
            const Node* node = *iter;
            const OutputNodeBase* outputNode = dynamic_cast<const OutputNodeBase*>(node);
            if (outputNode != nullptr)
            {
                result.push_back(outputNode);
            }
        }
        return result;
    }

    std::vector<const Node*> Map::GetSinkNodes() const
    {
        return GetMatchingNodesByType("SinkNode");
    }

    MemoryShape Map::GetOutputShape(size_t index) const
    {
        return GetOutput(index).GetMemoryLayout().GetActiveSize();
    }

    MemoryShape Map::GetSinkOutputShape(size_t index) const
    {
        const Node* node = GetSinkNode(index);
        return node->GetOutputPort(0)->GetMemoryLayout().GetActiveSize();
    }

    const Node* Map::GetSinkNode(size_t index) const
    {
        auto nodes = GetSinkNodes();

        if (index >= nodes.size())
        {
            throw utilities::InputException(utilities::InputExceptionErrors::indexOutOfRange);
        }

        return nodes[index];
    }

    Port::PortType Map::GetInputType(size_t index) const
    {
        // If we have SourceNodes then these override the InputNodes in terms of our compiled API.
        auto sourceNodes = _model.GetNodesByType<SourceNodeBase>();
        if (!sourceNodes.empty())
        {
            return sourceNodes[index]->GetOutputType();
        }

        return GetInput(index)->GetOutputType();
    }

    Port::PortType Map::GetOutputType(size_t index) const
    {
        return GetOutput(index).GetPortType();
    }

    Port::PortType Map::GetSinkOutputType(size_t index) const
    {
        return GetSinkNode(index)->GetOutputPort(0)->GetType();
    }

    void Map::Refine(int maxIterations)
    {
        TransformContext context;
        Refine(context, maxIterations);
    }

    void Map::Refine(const TransformContext& context, int maxIterations)
    {
        if (maxIterations == 0)
        {
            return;
        }

        RefineTransformation refineTransformation(maxIterations);
        Transform(refineTransformation, context);
        Prune();
    }

    void Map::Transform(const std::function<void(const Node&, ModelTransformer&)>& transformFunction)
    {
        TransformContext context;
        Transform(context, transformFunction);
    }

    void Map::Transform(const TransformContext& context, const std::function<void(const Node&, ModelTransformer&)>& transformFunction)
    {
        ModelTransformer transformer;
        auto newModel = transformer.TransformModel(_model, context, transformFunction);
        FixTransformedIO(transformer);
        _model = std::move(newModel);
    }

    void Map::Transform(Transformation& transformation)
    {
        TransformContext context;
        Transform(transformation, context);
    }

    void Map::Transform(Transformation& transformation, const TransformContext& context)
    {
        ModelTransformer transformer;
        auto newModel = transformation.TransformModel(_model, transformer, context);
        FixTransformedIO(transformer);
        _model = newModel.ShallowCopy();
    }

    void Map::RenameCallbacks(const std::string& sourceCallbackName, const std::string& sinkCallbackName)
    {
        // Look for all source nodes and apply name if it is non-empty.
        // The callbacks will soon support a context parameter that indicates the originating source node.
        if (!sourceCallbackName.empty())
        {
            auto nodes = _model.GetNodesByType<SourceNodeBase>();
            for (size_t i = 0; i < nodes.size(); ++i)
            {
                nodes[i]->SetCallbackName(sourceCallbackName);
            }
        }

        // Look for all sink nodes and apply name if it is non-empty.
        // The callbacks will soon support a context parameter that indicates the originating sink node.
        if (!sinkCallbackName.empty())
        {
            auto nodes = _model.GetNodesByType<SinkNodeBase>();
            for (size_t i = 0; i < nodes.size(); ++i)
            {
                nodes[i]->SetCallbackName(sinkCallbackName);
            }
        }
    }

    utilities::ArchiveVersion Map::GetArchiveVersion() const
    {
        if (_metadata.IsEmpty())
        {
            return noMetadataArchiveVersion;
        }
        else
        {
            return metadataArchiveVersion;
        }
    }

    bool Map::CanReadArchiveVersion(const utilities::ArchiveVersion& version) const
    {
        return version >= noMetadataArchiveVersion && version <= metadataArchiveVersion;
    }

    void Map::WriteToArchive(utilities::Archiver& archiver) const
    {
        // Archive the model
        archiver["model"] << _model;

        // Archive the inputs
        std::vector<utilities::UniqueId> inputIds = utilities::TransformVector(_inputNodes.begin(), _inputNodes.end(), [](InputNodeBase* node) { return node->GetId(); });

        archiver["inputNames"] << _inputNames;
        archiver["inputIds"] << inputIds;

        // Archive the outputs
        archiver["outputNames"] << _outputNames;
        archiver["outputElements"] << _outputElements;

        if (!_metadata.IsEmpty())
        {
            archiver["metadata"] << _metadata;
        }
    }

    void Map::ReadFromArchive(utilities::Unarchiver& archiver)
    {
        MapSerializationContext mapContext(archiver.GetContext());
        archiver.PushContext(mapContext);

        // Unarchive the model
        archiver["model"] >> _model;

        // Unarchive the inputs
        std::vector<utilities::UniqueId> inputIds;
        archiver["inputNames"] >> _inputNames;
        archiver["inputIds"] >> inputIds;

        // Unarchive the outputs
        archiver["outputNames"] >> _outputNames;
        archiver["outputElements"] >> _outputElements;

        if (archiver.HasNextPropertyName("metadata"))
        {
            archiver["metadata"] >> _metadata;
        }

        // Reconstruct the input node map
        _inputNodeMap.clear();
        _inputNodes.resize(inputIds.size());
        assert(_inputNames.size() == inputIds.size());
        for (size_t index = 0; index < _inputNames.size(); ++index)
        {
            auto node = mapContext.GetNodeFromSerializedId(inputIds[index]);
            assert(dynamic_cast<InputNodeBase*>(node) != nullptr);
            _inputNodes[index] = static_cast<InputNodeBase*>(node);
            _inputNodeMap[_inputNames[index]] = static_cast<InputNodeBase*>(node);
        }

        // Reconstruct the output elements map
        _outputElementsMap.clear();
        assert(_outputNames.size() == _outputElements.size());
        for (size_t index = 0; index < _outputNames.size(); ++index)
        {
            _outputElementsMap[_outputNames[index]] = _outputElements[index];
        }

        archiver.PopContext();
    }

    InputNodeBase* Map::GetInput(size_t index) const
    {
        if (index >= _inputNodes.size())
        {
            throw utilities::InputException(utilities::InputExceptionErrors::invalidArgument);
        }

        return _inputNodes[index];
    }

    InputNodeBase* Map::GetInput(const std::string& inputName) const
    {
        auto iter = _inputNodeMap.find(inputName);
        if (iter == _inputNodeMap.end())
        {
            throw utilities::InputException(utilities::InputExceptionErrors::invalidArgument);
        }

        return iter->second;
    }

    std::string Map::GetInputName(size_t index) const
    {
        if (index >= _inputNames.size())
        {
            throw utilities::InputException(utilities::InputExceptionErrors::invalidArgument);
        }

        return _inputNames[index];
    }

    PortElementsBase Map::GetOutput(size_t index) const
    {
        if (index >= _outputElements.size())
        {
            throw utilities::InputException(utilities::InputExceptionErrors::invalidArgument);
        }

        return _outputElements[index];
    }

    PortElementsBase Map::GetOutput(const std::string& outputName) const
    {
        auto iter = _outputElementsMap.find(outputName);
        if (iter == _outputElementsMap.end())
        {
            throw utilities::InputException(utilities::InputExceptionErrors::invalidArgument);
        }

        return iter->second;
    }

    std::string Map::GetOutputName(size_t index) const
    {
        if (index >= _outputElements.size())
        {
            throw utilities::InputException(utilities::InputExceptionErrors::invalidArgument);
        }

        return _outputNames[index];
    }

    //
    // MapSerializationContext
    //
    MapSerializationContext::MapSerializationContext(utilities::SerializationContext& previousContext) :
        ModelSerializationContext(previousContext, nullptr)
    {
    }
} // namespace model
} // namespace ell
