/**
 * @file ParameterSettings.h
 * @brief class ParameterSettings
 *
 * @author Matthias Schartner
 * @date 23.08.17
 */

#ifndef PARAMETERSETTINGS_H
#define PARAMETERSETTINGS_H

#include <ostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/date_time.hpp>
#include <utility>
#include <unordered_map>

#include "ParameterSetup.h"
#include "ParameterGroup.h"

namespace VieVS {
    /**
     * @class ParameterSettings
     * @brief This class stores all setup information and writes a parameters.xml file
     *
     * @author Matthias Schartner
     * @date 22.08.2017
     */
    class ParameterSettings {
    public:

        /**
         * @brief possible setup and optimization flags
         */
        enum class Type {
            station, ///< stations wise group/setup
            source, ///< source wise group/setup
            baseline, ///< baseline wise group/setup
        };


        /**
         * @brief station parameters
         */
        struct ParametersStations{
            boost::optional<bool> firstScan; ///< if set to true: no time is spend for setup, source, tape, calibration, and slewing
            boost::optional<bool> available;  ///< if set to true: this station is available for a scan
            boost::optional<bool> tagalong;  ///< if set to true: station is in tagalong mode

            std::unordered_map<std::string, double> minSNR; ///< minimum required signal to noise ration for each band

            boost::optional<unsigned int> maxSlewtime; ///< maximum allowed slewtime
            boost::optional<unsigned int> maxWait; ///< maximum allowed wait time for slow antennas
            boost::optional<unsigned int> maxScan; ///< maximum allowed scan time
            boost::optional<unsigned int> minScan; ///< minimum required scan time

            boost::optional<double> weight; ///< multiplicative factor of score for scans with this station

            std::vector<std::string> ignoreSources_str; ///< list of all source names which should be ignored
        };

        /**
         * @brief source parameters
         */
        struct ParametersSources {
            boost::optional<bool> available = true; ///< flag is source is available

            boost::optional<double> weight = 1; ///< multiplicative factor of score for scans to this source

            std::unordered_map<std::string, double> minSNR; ///< minimum required signal to noise ration for each band

            boost::optional<unsigned int> minNumberOfStations = 2; ///< minimum number of stations for a scan
            boost::optional<double> minFlux = .01; ///< minimum flux density required for this source in jansky
            boost::optional<unsigned int> minRepeat = 1800; ///< minimum time between two observations of this source in seconds
            boost::optional<unsigned int> maxScan = 600; ///< maximum allowed scan time in seconds
            boost::optional<unsigned int> minScan = 30; ///< minimum required scan time in seconds
            boost::optional<unsigned int> maxNumberOfScans = 9999; ///< maximum number of scans
            boost::optional<bool> tryToFocusIfObservedOnce = false; ///< flag if this source should be focused after observed once

            boost::optional<unsigned int> tryToObserveXTimesEvenlyDistributed; ///< tries to observe a source X times over the timespan in which the source is scanable. Overwrites maxScan and tryToFocusIfObservedOnce.
            boost::optional<unsigned int> fixedScanDuration; ///< optional fixed scan duration

            std::vector<int> ignoreStations; ///< list of all stations ids which should be ignored
            std::vector<std::string> ignoreStationsString; ///< list of all station names which should be ignored
            std::vector<std::pair<int, int>> ignoreBaselines; ///< list of all baseline ids which should be ignored
            std::vector<std::pair<std::string, std::string>> ignoreBaselinesString; ///< list of all baseline names which should be ignore
            std::vector<int> requiredStations; ///< list of station ids which are required for a scan to this source
            std::vector<std::string> requiredStationsString; ///< list of station names which are required for a scan to this source
        };

        /**
         * @brief baseline parameters
         */
        struct ParametersBaselines {
            std::unordered_map<std::string, double> minSNR; ///< minimum SNR per band for each baseline
            boost::optional<bool> ignore; ///< ignore specific baselines

            boost::optional<double> weight; ///< multiplicative factor of score for scans with this baseline
            boost::optional<unsigned int> minScan; ///< minimum required scan duration of this baseline
            boost::optional<unsigned int> maxScan; ///< maximum allowed scan duration of this baseline
        };


        /**
         * @brief all possible observation mode flux information type
         */
        enum class ObservationModeProperty {
            required,    ///< this band information is required. If this information is missing this object is not used.
            optional,    ///< this band information is only optional. If information is available it is used, otherwise it is interpolated
        };

        /**
        * @brief all possible observation mode backup models
        */
        enum class ObservationModeBackup {
            minValueTimes, ///< use minimum value found in other bands times a factor
            maxValueTimes, ///< use maximum value found in other bands times a factor
            value, ///< use specific value
            none, ///< no backup model
        };



        /**
         * @brief empty default constructor
         */
        ParameterSettings();

        /**
         * @brief software block in parameter.xml
         *
         * @param name software name
         * @param version software version
         * @param created local time and date of created parameter.xml file
         */
        void software(const std::string &name, const std::string &version, const boost::posix_time::ptime &created);

        /**
         * @brief general block in parameter.xml
         *
         * @param startTime session start time
         * @param endTime session end time
         * @param subnetting flag if subnetting is allowed
         * @param fillinmode flag if fillin modes are allowed
         * @param fillinmodeInfluence flag if fillin mode scans should have an influence on the schedule
         * @param stations list of all stations
         */
        void general(const boost::posix_time::ptime &startTime, const boost::posix_time::ptime &endTime,
                     bool subnetting, bool fillinmode, bool fillinmodeInfluenceOnSchedule, double minElevation,
                     const std::vector<std::string> &stations);

        /**
         * @brief catalogs block in parameters.xml
         *
         * @param antenna antenna catalog name
         * @param equip equip catalog name
         * @param flux flux catalog name
         * @param freq freq catalog name
         * @param hdpos hdpos catalog name
         * @param loif loif catalog name
         * @param mask mask catalog name
         * @param modes modes catalog name
         * @param position position catalog name
         * @param rec rec catalog name
         * @param rx rx catlog name
         * @param source source catalog name
         */
        void
        catalogs(const std::string &antenna, const std::string &equip, const std::string &flux, const std::string &freq,
                 const std::string &hdpos, const std::string &loif, const std::string &mask, const std::string &modes,
                 const std::string &position, const std::string &rec, const std::string &rx, const std::string &source,
                 const std::string &tracks);

        /**
         * @brief group defined in parameters.xml
         *
         * @param type group type
         * @param group group object
         */
        void group(Type type, ParameterGroup group);

        /**
         * @brief returns all members of a defined group
         *
         * @param type group type
         * @param groupName group name
         * @return list of all group members
         */
        const std::vector<std::string> &getGroupMembers(Type type, std::string groupName);

        /**
         * @brief defined station parameters in station block in parameters.xml
         *
         * @param name parameter name
         * @param PARA parameters
         */
        void parameters(const std::string &name, ParametersStations PARA);

        /**
         * @brief defined source parameters in source block in parameters.xml
         *
         * @param name parameter name
         * @param PARA parameters
         */
        void parameters(const std::string &name, ParametersSources PARA);

        /**
         * @brief defined baseline parameters in baseline block in parameters.xml
         *
         * @param name parameter name
         * @param PARA parameters
         */
        void parameters(const std::string &name, ParametersBaselines PARA);

        /**
         * @brief setup block in parameter.xml
         *
         * @param type flag where this setup belongs
         * @param setup setup object
         */
        void setup(Type type, const ParameterSetup &setup);

        /**
         * @brief station wait times block
         *
         * @param name group, station name or "__all__"
         * @param setup time for setup in seconds
         * @param source time for source command in seconds
         * @param tape time for tape command in seconds
         * @param calibration time for preobservation in seconds
         * @param corsynch time for correlator synchronization
         */
        void stationWaitTimes(const std::string &name, unsigned int setup, unsigned int source, unsigned int tape,
                              unsigned int calibration, unsigned int corsynch);


        void stationCableWrapBuffer(const std::string &name, double axis1LowOffset, double axis1UpOffset,
                                    double axis2LowOffset, double axis2UpOffset);

        /**
         * @brief skyCoverage block in parameter.xml
         *
         * @param influenceDistance maximum angular influence distance in degrees
         * @param influenceInterval maximum time influence distance in seconds
         * @param maxTwinTelecopeDistance maximum distance between twin telescopes
         */
        void skyCoverage(double influenceDistance, unsigned int influenceInterval, double maxTwinTelecopeDistance);

        /**
         * @brief weightFactor block in parameter.xml
         *
         * @param weight_skyCoverage weight of sky Coverage
         * @param weight_numberOfObservations weight of number of observations
         * @param weight_duration weight of duration of the scan
         * @param weight_averageSources weight of an average source observation distribution
         * @param weight_averageStations weight of an average station usage distribution
         * @param weightDeclination weight factor for declination
         * @param declinationSlopeStart start declination of additional weight (everything above has factor 0)
         * @param declinationSlopeEnd end declination of additional declination weight slope (everything below has factor 1)
         * @param weightLowElevation weight factor for low elevation scans
         * @param lowElevationSlopeStart start elevation of additional weight (everything above has factor 0)
         * @param lowElevationSlopeEnd end elevation of additional declination weight slope (everything below has factor 1)
         */
        void weightFactor(double weight_skyCoverage, double weight_numberOfObservations, double weight_duration,
                          double weight_averageSources, double weight_averageStations, double weightDeclination,
                          double declinationSlopeStart, double declinationSlopeEnd, double weightLowElevation,
                          double lowElevationSlopeStart, double lowElevationSlopeEnd);

        /**
         * @brief custom mode block in parameter.xml
         *
         * @param sampleRate sample rate
         * @param bits bits
         */
        void mode(double sampleRate, unsigned int bits);

        /**
         * @brief mode block in parameter.xml
         * @param skdMode name of observing mode in skd catalogs
         */
        void mode(const std::string & skdMode);

        /**
         * @brief band sub-block in mode block in parameter.xml
         *
         * @param name band name
         * @param wavelength band wavelength
         * @param station station policy for this band
         * @param stationBackup station backup model
         * @param stationBackupValue station backup model value
         * @param source source policy for this band
         * @param sourceBackup source backup model
         * @param sourceBackupValue source backup model value
         * @param chanels number of channels
         */
        void mode_band(const std::string &name, double wavelength, ObservationModeProperty station,
                       ObservationModeBackup stationBackup, double stationBackupValue, ObservationModeProperty source,
                       ObservationModeBackup sourceBackup, double sourceBackupValue, unsigned int chanels);

        /**
         * @brief multisched block in parameter.xml
         *
         * @param multiSched multisched xml tree
         */
        void multisched(const boost::property_tree::ptree &multiSched);

        /**
         * @brief output routine that produces .xml file
         *
         * @param name output file name (usually "parameter")
         */
        void write(const std::string &name);

        /**
         * @brief output block in parameter.xml
         *
         * @param experimentName experiment name
         * @param experimentDescription experiment description
         * @param scheduler scheduler
         * @param correlator correlator
         * @param createSummary create summary file
         * @param createNGS create NGS file
         * @param createSKD create SKD file
         * @param createSkyCoverage create sky coverage file
         */
        void output(const std::string &experimentName, const std::string &experimentDescription,
                    const std::string &scheduler,
                    const std::string &correlator, bool createSummary, bool createNGS, bool createSKD,
                    bool createSkyCoverage);


        /**
         * @brief specify the sequence in which sources should be observed
         *
         * If you say cadence = 5, modulo = 0, source = A than every 5th selected scan(or subnetting sequence) will be
         * (if possible) to a source of A. This does not effect the first scan of a schedule!
         *
         * You can use the modulo number to to combine different rules:
         * cadence = 10, modulo = 0, source = "1234+567" and cadence = 10, modulo = 1, source = "1234+568" means that
         * every 10th scan will be (if possible) to source 1234+567 followed by a scan to "1234+568".
         *
         * @param cadence cadence for this rule
         * @param modulo modulo operator for number of scan and cadence
         * @param member source(group) which should be observed
         */
        void ruleScanSequence(unsigned int cadence, const std::vector<unsigned int> &modulo,
                              const std::vector<std::string> &member);

        void ruleCalibratorBlockTime(unsigned int cadence, const std::string &member,
                                     const std::vector<std::pair<double,double> > &between_elevation,
                                     unsigned int nMaxScans, unsigned int scanTime);

        void ruleCalibratorBlockNScanSelections(unsigned int cadence, const std::string &member,
                                                unsigned int nMaxScans, unsigned int scanTime);

        const std::map<std::string, std::vector<std::string>> &getGroupStations() const {
            return groupStations_;
        }

        const std::map<std::string, std::vector<std::string>> &getGroupSources() const {
            return groupSources_;
        }

        const std::map<std::string, std::vector<std::string>> &getGroupBaselines() const {
            return groupBaselines_;
        }

        const std::map<std::string, ParametersStations> &getParaStations() const {
            return paraStations_;
        }

        const std::map<std::string, ParametersSources> &getParaSources() const {
            return paraSources_;
        }

        const std::map<std::string, ParametersBaselines> &getParaBaselines() const {
            return paraBaselines_;
        }



    private:
        boost::property_tree::ptree master_; ///< master property tree

        std::map<std::string, std::vector<std::string> > groupStations_; ///< defined station group
        std::map<std::string, std::vector<std::string> > groupSources_; ///< defined source group
        std::map<std::string, std::vector<std::string> > groupBaselines_; ///< defined baseline group

        std::map<std::string, ParametersStations > paraStations_; ///< defined station parameters
        std::map<std::string, ParametersSources > paraSources_; ///< defined source parameters
        std::map<std::string, ParametersBaselines > paraBaselines_; ///< defined baseline parameters

        /**
         * @brief returns a child tree of root setup object
         *
         * @param setup root setup object
         * @return child property tree
         */
        boost::property_tree::ptree getChildTree(const ParameterSetup &setup);
    };
}


#endif //PARAMETERSETTINGS_H
