#include <gui/smgui.h>
#include <cstdio>
#include <module.h>
#include <gui/gui.h>
#include <signal_path/signal_path.h>
#include <core.h>
#include <string>
#include <thread>
#include <filesystem>


SDRPP_MOD_INFO{
    /* Name:            */ "cxadc_source",
    /* Description:     */ "CXADC Source for SDR++",
    /* Author:          */ "Shadowstrike114",
    /* Version:         */ 0, 0, 1,
    /* Max instances    */ 1
};

class CXADCSourceModule : public ModuleManager::Instance{
public:
    CXADCSourceModule(std::string name){
        this->name = name;

        samplerate = 40000000;

        handler.ctx = this;
        handler.selectHandler = menuSelected;
        handler.deselectHandler = menuDeselected;
        handler.menuHandler = menuHandler;
        handler.startHandler = start;
        handler.stopHandler = stop;
        handler.tuneHandler = tune;
        handler.stream = &stream;

        this->refresh(this);

        sigpath::sourceManager.registerSource("CXADC", &handler);
    }

    ~CXADCSourceModule() {
        stop(this);
    }

    void postInit() {}

    void enable() {
        enabled = true;
    }

    void disable() {
        enabled = false;
    }

    bool isEnabled() {
        return enabled;
    }

private:
    std::string name;
    bool enabled = true;
    dsp::stream<dsp::complex_t> stream;
    SourceManager::SourceHandler handler;
    bool running = false;
    double freq;
    std::thread workerThread;

    const std::filesystem::path cxadc_dir = "/dev";
    std::vector<std::string> devList;
    std::string devListString;
    int devID=0;
    int samplerate = 40000000;
    int center_offset = 0;
    int level = 16;
    int vmux = 0;
    bool sixdb = false;
    std::string cxadc_path = "/dev/cxadc0";
    std::string cxadc_settings_path = "/sys/class/cxadc/cxadc0/device/parameters/";
    FILE*  file;



    static void menuSelected(void* ctx) {
        CXADCSourceModule* _this = (CXADCSourceModule*)ctx;
        core::setInputSampleRate(_this->samplerate);
        flog::info("CXADCSourceModule '{0}': Menu Select!", _this->name);
    }

    static void menuDeselected(void* ctx) {
        CXADCSourceModule* _this = (CXADCSourceModule*)ctx;
        flog::info("CXADCSourceModule '{0}': Menu Deselect!", _this->name);
    }

    static void menuHandler(void* ctx) {
        CXADCSourceModule* _this = (CXADCSourceModule*)ctx;

        //TODO:
        //  - Which CXADC Device
        //  - Sample type selector
        //      8 or 10 bit
        // - Samplerate selector
        //      what is the crystal
        // - VMUX Selector
        //      which of the three inputs
        // - Offset Selector
        // - Gain Selector
        // - Sixdb selector
  

        if (_this->running) { SmGui::BeginDisabled(); }

        SmGui::ForceSync();
        //Erste Zeile: DeviceListe und refreshButton
        if (SmGui::Combo( (std::string("##_cxadc_select_") +  _this->name).c_str(), &_this->devID, _this->devListString.c_str())){
            _this->cxadc_path = std::string("/dev/") + _this->devList.at(_this->devID);
        }

        SmGui::SameLine();
        SmGui::FillWidth();
        SmGui::ForceSync();

        if (SmGui::Button((std::string("Refresh##_cxadc_refresh_") +  _this->name).c_str())) {
            _this->refresh(ctx);
        }

        SmGui::LeftLabel("Crystal");
        SmGui::FillWidth();
        if (SmGui::InputInt((std::string("Crystal##_cxadc_samplerate_") +  _this->name).c_str(), &_this->samplerate, 1000000, 1000000)){

        }

        SmGui::LeftLabel("VMUX");
        SmGui::FillWidth();
        if (SmGui::SliderInt((std::string("vmux##_cxadc_vmux_") +  _this->name).c_str(), &_this->vmux, 0, 2)){
            _this->cxadc_settings_path = std::string("/sys/class/cxadc/") + _this->devList.at(_this->devID) + std::string("/device/parameters/");
            FILE *f;
            f = fopen((_this->cxadc_settings_path + std::string("vmux")).c_str(), "w");
            if (NULL != f) {
                fputs(std::to_string(_this->vmux).c_str(), f);
                fclose(f);
            }
        }

        if (_this->running) { SmGui::EndDisabled(); }

        //live controls
        SmGui::LeftLabel("center_offset");
        SmGui::FillWidth();
        if (SmGui::SliderInt((std::string("center_offset##_cxadc_center_offset_") +  _this->name).c_str(), &_this->center_offset, 0, 255)){
            _this->cxadc_settings_path = std::string("/sys/class/cxadc/") + _this->devList.at(_this->devID) + std::string("/device/parameters/");
            FILE *f;
            f = fopen((_this->cxadc_settings_path + std::string("center_offset")).c_str(), "w");
            if (NULL != f) {
                fputs(std::to_string(_this->center_offset).c_str(), f);
                fclose(f);
            }
        }

        SmGui::LeftLabel("Level");
        SmGui::FillWidth();
        if (SmGui::SliderInt((std::string("Level##_cxadc_level_") +  _this->name).c_str(), &_this->level, 0, 31)){
            _this->cxadc_settings_path = std::string("/sys/class/cxadc/") + _this->devList.at(_this->devID) + std::string("/device/parameters/");
            FILE *f;
            f = fopen((_this->cxadc_settings_path + std::string("level")).c_str(), "w");
            if (NULL != f) {
                fputs(std::to_string(_this->level).c_str(), f);
                fclose(f);
            }
        }

        SmGui::LeftLabel("Sixdb");
        SmGui::ForceSync();
        if (SmGui::Checkbox((std::string("+6 dB##_cxadc_sixdb_") +  _this->name).c_str(), &_this->sixdb)){
            _this->cxadc_settings_path = std::string("/sys/class/cxadc/") + _this->devList.at(_this->devID) + std::string("/device/parameters/");
            FILE *f;
            f = fopen((_this->cxadc_settings_path + std::string("sixdb")).c_str(), "w");
            if (NULL != f) {
                fputs(std::to_string((_this->sixdb == true)?1:0).c_str(), f);
                fclose(f);
            }
        }

    }


    static void start(void* ctx){
        CXADCSourceModule* _this = (CXADCSourceModule*)ctx;
        if (_this->running) { return; }

        _this->file = fopen(_this->cxadc_path.c_str(), "r");

        if (NULL == _this->file){ return; }
        
        _this->workerThread = std::thread(worker_8bit,_this);

        _this->running = true;
        flog::info("CXADCSourceModule '{0}': Start!", _this->name);
    }


    static void tune(double freq, void* ctx) {
        CXADCSourceModule* _this = (CXADCSourceModule*)ctx;
            // We cant tune since we just sample at full rate --> do nothing
        flog::info("CXADCSourceModule '{0}': Trying to tune to: {1}!", _this->name, freq);
    }


    static void stop(void* ctx) {
        CXADCSourceModule* _this = (CXADCSourceModule*)ctx;
        if (!_this->running) { return; }
        flog::info("CXADCSourceModule '{0}': Stopping...", _this->name);

        _this->stream.stopWriter();
        if (_this->workerThread.joinable()) { _this->workerThread.join(); }
        _this->stream.clearWriteStop();

        fclose(_this->file);

        _this->running = false;
        flog::info("CXADCSourceModule '{0}': Stop!", _this->name);
    }

    static void worker_8bit(void* ctx){
        CXADCSourceModule* _this = (CXADCSourceModule*)ctx;
        if (NULL == _this->file){ return; }

        uint64_t sample = 0; 

        uint8_t rbuf[65536*2];

        while (true){
            size_t bytes_read = fread(rbuf,1,65536*2,_this->file);            

            if ( 0 != bytes_read){ 
                for (size_t i = 0; i < bytes_read; i++ ){            
                    _this->stream.writeBuf[i].re = ((float)rbuf[i] - 127.4f) / 128.0f;
                    _this->stream.writeBuf[i].im = 0.0f;
                }               
                if (!_this->stream.swap(bytes_read)) { break; };
            }
        }  
    }

    static void refresh(void * ctx){
        CXADCSourceModule* _this = (CXADCSourceModule*)ctx;

        _this->devList.clear();
        _this->devListString = "";

        //find all devices in /dev/cxadc
        for (auto const& entry : std::filesystem::directory_iterator{_this->cxadc_dir}){
            
            std::string name = entry.path().filename().string();

            if (name.find("cxadc") != std::string::npos){
                //Device is a CXADC Device --> add it to the list
                _this->devList.push_back(name);
                _this->devListString += name;
                _this->devListString += '\0';
            }
        }
    }
};

MOD_EXPORT void _INIT_() {

}

MOD_EXPORT ModuleManager::Instance* _CREATE_INSTANCE_(std::string name) {
    return new CXADCSourceModule(name);
}

MOD_EXPORT void _DELETE_INSTANCE_(ModuleManager::Instance* instance) {
    delete (CXADCSourceModule*)instance;
}

MOD_EXPORT void _END_() {

}
