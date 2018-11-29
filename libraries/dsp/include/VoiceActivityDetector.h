////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Learning Library (ELL)
//  File:     VoiceActivityDetector.h (dsp)
//  Authors:  Chris Lovett, Ivan Tashev
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utilities/include/Archiver.h>
#include <utilities/include/Exception.h>
#include <utilities/include/IArchivable.h>
#include <utilities/include/TypeName.h>

#include <cmath>
#include <complex>
#include <math.h>
#include <memory>
#include <vector>

namespace ell
{
namespace dsp
{
    struct VoiceActivityDetectorImpl;

    /// <summary> This class detects speech activity in an audio stream </summary>
    class VoiceActivityDetector : public utilities::IArchivable
    {
        std::unique_ptr<VoiceActivityDetectorImpl> _impl;

    public:
        /// <summary>
        /// default constructor, used only during deserialization
        /// </summary>
        VoiceActivityDetector();

        /// <summary> Construct new voice activity recorder  </summary>
        ///
        /// <param name="sampleRate"> The sample rate of input signal. </param>
        /// <param name="windowSize"> The size of the window (should equal the size of the data given to each process call). </param>
        /// <param name="frameDuration"> The frames duration (inverse of frames per second), this is not necessarily the same as windowSize / sampleRate, it also depends on the 'shift'. </param>
        /// <param name="tauUp"> The noise floor is computed by tracking the frame power.  It goes up slow, with this time constant tauUp. </param>
        /// <param name="tauDown"> If the frame power is lower than the noise floor, it goes down fast, with this time constant tauDown. </param>
        /// <param name="largeInput"> The exception is the case when the proportion frame power/noise floor is larger than this largeInput. </param>
        /// <param name="gainAtt"> Then we switch to much slower adaptation by applying this gainAtt. </param>
        /// <param name="thresholdUp"> Then we compare the energy of the current frame to the noise floor. If it is thresholdUp times higher, we switch to state VOICE. </param>
        /// <param name="thresholdDown"> Then we compare the energy of the current frame to the noise floor. If it is thresholdDown times lower, we switch to state NO VOICE.  </param>
        /// <param name="levelThreshold"> Special case is when the energy of the frame is lower than levelThreshold, when we force the state to NO VOICE. </param>
        VoiceActivityDetector(
            double sampleRate,
            double windowSize,
            double frameDuration,
            double tauUp,
            double tauDown,
            double largeInput,
            double gainAtt,
            double thresholdUp,
            double thresholdDown,
            double levelThreshold);

        /// <summary> destructor </summary>
        ~VoiceActivityDetector();

        /// <summary> reset the activity detector so it can be used on a new stream </summary>
        void reset();

        /// <summary> process incoming audio stream, this data should already be floating point in the range[0 - 1].
        /// This method returns 1 when it detects activity in the stream and 0 otherwise. </summary>
        ///
        /// <param name="data"> The input signal. </param>
        template <typename ValueType>
        int process(const std::vector<ValueType>& data);

        /// <summary> return true if the two detectors have the same sample rate and window size </summary>
        bool equals(const VoiceActivityDetector& other) const;

        /// <summary> Gets the name of this type. </summary>
        ///
        /// <returns> The name of this type. </returns>
        static std::string GetTypeName() { return "VoiceActivityDetector"; }

        /// <summary> Gets the name of this type (for serialization). </summary>
        ///
        /// <returns> The name of this type. </returns>
        std::string GetRuntimeTypeName() const override { return GetTypeName(); }

        /// <summary> internal use only: used for testing </summary>
        const std::vector<double>& getWeights() const;

        /// <summary> Get the sample rate provided to constructor </summary>
        double getSampleRate() const;

        /// <summary> Get the window size provided to constructor </summary>
        double getWindowSize() const;

        /// <summary> Get the frame duration provided to constructor </summary>
        double getFrameDuration() const;

        /// <summary> Get the tauUp parameter provided to constructor </summary>
        double getTauUp() const;

        /// <summary> Get the tauDown parameter provided to constructor </summary>
        double getTauDown() const;

        /// <summary> Get the largeInput parameter provided to constructor </summary>
        double getLargeInput() const;

        /// <summary> Get the gainAtt parameter provided to constructor </summary>
        double getGainAtt() const;

        /// <summary> Get the thresholdUp parameter provided to constructor </summary>
        double getThresholdUp() const;

        /// <summary> Get the thresholdDown parameter provided to constructor </summary>
        double getThresholdDown() const;

        /// <summary> Get the levelThreshold parameter provided to constructor </summary>
        double getLevelThreshold() const;

    protected:
        void WriteToArchive(utilities::Archiver& archiver) const override;
        void ReadFromArchive(utilities::Unarchiver& archiver) override;
    };
} // namespace dsp
} // namespace ell
