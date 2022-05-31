#include "network_request.hpp"

tristan::network::NetworkRequest::NetworkRequest(tristan::network::Url&& url) :
        m_url(std::move(url)),
        m_uuid(utility::getUuid()),
        m_bytes_to_read(0),
        m_bytes_read(0),
        m_status(Status::WAITING),
        m_priority(Priority::NORMAL),
        m_paused(false),
        m_canceled(false),
        m_output_to_file(false){
    if (!m_url.isValid()){
        m_status = tristan::network::Status::ERROR;
        m_error = tristan::network::makeError(tristan::network::ErrorCode::INVALID_URL);
    }
}

tristan::network::NetworkRequest::NetworkRequest(const tristan::network::Url& url) :
        m_url(url),
        m_uuid(utility::getUuid()),
        m_bytes_to_read(0),
        m_bytes_read(0),
        m_status(Status::WAITING),
        m_priority(Priority::NORMAL),
        m_paused(false),
        m_canceled(false),
        m_output_to_file(false){
    if (!m_url.isValid()){
        m_status = tristan::network::Status::ERROR;
        m_error = tristan::network::makeError(tristan::network::ErrorCode::INVALID_URL);
    }
}

void tristan::network::NetworkRequest::setPriority(tristan::network::Priority priority){
    m_priority = priority;
}

void tristan::network::NetworkRequest::outputToFile(std::filesystem::path&& path){
    m_output_path = std::move(path);
    m_output_to_file = true;
}

void tristan::network::NetworkRequest::outputToFile(const std::filesystem::path& path){
    m_output_path = path;
    m_output_to_file = true;
}

void tristan::network::NetworkRequest::cancel(){
    m_canceled.store(true, std::memory_order_relaxed);
}

void tristan::network::NetworkRequest::pauseProcessing(){
    m_paused.store(true, std::memory_order_relaxed);
}

void tristan::network::NetworkRequest::continueProcessing(){
    m_paused.store(false, std::memory_order_relaxed);
}

auto tristan::network::NetworkRequest::uuid() const noexcept -> const std::string&{
    return m_uuid;
}

auto tristan::network::NetworkRequest::url() const noexcept -> const tristan::network::Url&{
    return m_url;
}

auto tristan::network::NetworkRequest::error() const noexcept -> const std::error_code&{
    return m_error;
}

auto tristan::network::NetworkRequest::status() const noexcept -> tristan::network::Status{
    return m_status;
}

auto tristan::network::NetworkRequest::isPaused() const noexcept -> bool{
    return m_paused.load(std::memory_order_relaxed);
}

auto tristan::network::NetworkRequest::isCanceled() const noexcept -> bool{
    return m_canceled.load(std::memory_order_relaxed);
}

auto tristan::network::NetworkRequest::priority() const noexcept -> tristan::network::Priority{
    return m_priority;
}

auto tristan::network::NetworkRequest::requestData() const -> const std::vector<uint8_t>&{
    return m_request_data;
}

void tristan::network::NetworkRequest::setRequest(std::vector<uint8_t>&& request_data){
    m_request_data = std::move(request_data);
}

void tristan::network::NetworkRequest::setRequest(const std::vector<uint8_t>& request_data){
    m_request_data = request_data;
}

auto tristan::network::NetworkRequest::responseData() -> std::shared_ptr<std::vector<uint8_t>>{
    return m_response_data;
}

void tristan::network::NetworkRequest::addReadBytesValueChangedCallback(std::function<void(uint64_t)>&& functor){
    m_read_bytes_changed_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addReadBytesValueChangedCallback(std::function<void(const std::string&, uint64_t)>&& functor){
    m_read_bytes_changed_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addFinishedCallback(std::function<void()>&& functor){
    m_finished_void_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addFinishedCallback(std::function<void(const std::string&)>&& functor){
    m_finished_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addFinishedCallback(std::function<void(std::shared_ptr<std::vector<uint8_t>>)>&& functor){
    m_finished_with_response_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addFinishedCallback(std::function<void(const std::string&, std::shared_ptr<std::vector<uint8_t>>)>&& functor){
    m_finished_with_id_and_response_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addStatusChangedCallback(std::function<void()>&& functor){
    m_status_changed_void_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addStatusChangedCallback(std::function<void(const std::string&)>&& functor){
    m_status_changed_with_id_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addStatusChangedCallback(std::function<void(Status)>&& functor){
    m_status_changed_with_status_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addStatusChangedCallback(std::function<void(const std::string&, Status)>&& functor){
    m_status_changed_with_id_and_status_callback_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addPausedCallback(std::function<void()>&& functor){
    m_paused_void_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addPausedCallback(std::function<void(const std::string&)>&& functor){
    m_paused_with_id_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addResumedCallback(std::function<void()>&& functor){
    m_resumed_void_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addResumedCallback(std::function<void(const std::string&)>&& functor){
    m_resumed_with_id_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addCanceledCallback(std::function<void()>&& functor){
    m_canceled_void_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::addCanceledCallback(std::function<void(const std::string&)>&& functor){
    m_canceled_with_id_functors.emplace_back(std::move(functor));
}

void tristan::network::NetworkRequest::pNotifyWhenBytesReadChanged(){
    if (!m_read_bytes_changed_callback_functors.empty()){
        for (const auto& functor: m_read_bytes_changed_callback_functors){
            functor(m_bytes_read);
        }
    }
    if (!m_read_bytes_changed_with_id_callback_functors.empty()){
        for (const auto& functor: m_read_bytes_changed_with_id_callback_functors){
            functor(m_uuid, m_bytes_read);
        }
    }
}

void tristan::network::NetworkRequest::pNotifyWhenStatusChanged(){
    if (!m_status_changed_void_callback_functors.empty()){
        for (const auto& functor: m_status_changed_void_callback_functors){
            functor();
        }
    }
    if (!m_status_changed_with_id_callback_functors.empty()){
        for (const auto& functor: m_status_changed_with_id_callback_functors){
            functor(m_uuid);
        }
    }
    if (!m_status_changed_with_status_callback_functors.empty()){
        for (const auto& functor: m_status_changed_with_status_callback_functors){
            functor(m_status);
        }
    }
    if (!m_status_changed_with_id_and_status_callback_functors.empty()){
        for (const auto& functor: m_status_changed_with_id_and_status_callback_functors){
            functor(m_uuid, m_status);
        }
    }
}

void tristan::network::NetworkRequest::pNotifyWhenPaused(){
    if (!m_paused_void_functors.empty()){
        for (const auto& functor: m_paused_void_functors){
            functor();
        }
    }
    if (!m_paused_with_id_functors.empty()){
        for (const auto& functor: m_paused_with_id_functors){
            functor(m_uuid);
        }
    }
}

void tristan::network::NetworkRequest::pNotifyWhenResumed(){
    if (!m_resumed_void_functors.empty()){
        for (const auto& functor: m_resumed_void_functors){
            functor();
        }
    }
    if (!m_resumed_with_id_functors.empty()){
        for (const auto& functor: m_resumed_with_id_functors){
            functor(m_uuid);
        }
    }
}

void tristan::network::NetworkRequest::pNotifyWhenCanceled(){
    if (!m_canceled_void_functors.empty()){
        for (const auto& functor: m_canceled_void_functors){
            functor();
        }
    }
    if (!m_canceled_with_id_functors.empty()){
        for (const auto& functor: m_canceled_with_id_functors){
            functor(m_uuid);
        }
    }
}

void tristan::network::NetworkRequest::pNotifyWhenFinished(){
    if (!m_finished_void_callback_functors.empty()){
        for (const auto& functor: m_finished_void_callback_functors){
            functor();
        }
    }
    if (!m_finished_with_id_callback_functors.empty()){
        for (const auto& functor: m_finished_with_id_callback_functors){
            functor(m_uuid);
        }
    }
    if (!m_finished_with_response_callback_functors.empty()){
        for (const auto& functor: m_finished_with_response_callback_functors){
            functor(m_response_data);
        }
    }
    if (!m_finished_with_id_and_response_callback_functors.empty()){
        for (const auto& functor: m_finished_with_id_and_response_callback_functors){
            functor(m_uuid, m_response_data);
        }
    }
}

void tristan::network::NetworkRequest::pNotifyWhenFailed(){
    if (!m_failed_void_callback_functors.empty()){
        for (const auto& functor: m_failed_void_callback_functors){
            functor();
        }
    }
    if (!m_failed_with_id_callback_functors.empty()){
        for (const auto& functor: m_failed_with_id_callback_functors){
            functor(m_uuid);
        }
    }
    if (!m_failed_with_error_code_callback_functors.empty()){
        for (const auto& functor: m_failed_with_error_code_callback_functors){
            functor(m_error);
        }
    }
    if (!m_failed_with_id_and_error_code_callback_functors.empty()){
        for (const auto& functor: m_failed_with_id_and_error_code_callback_functors){
            functor(m_uuid, m_error);
        }
    }
}

void tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(const std::shared_ptr<NetworkRequest>& network_request, std::vector<uint8_t>&& data){
    if (!network_request->m_output_to_file){
        if (!network_request->m_response_data){
            network_request->m_response_data = std::make_shared<std::vector<uint8_t>>(std::move(data));
        }
        else{
            network_request->m_response_data->insert(network_request->m_response_data->end(), data.begin(), data.end());
        }
    }
    else{
        if (network_request->m_output_path.empty()){
            pSetError(network_request, tristan::network::makeError(tristan::network::ErrorCode::FILE_PATH_EMPTY));
            return;
        }
        if (!std::filesystem::exists(network_request->m_output_path)){
            if (!std::filesystem::exists(network_request->m_output_path.parent_path())){
                pSetError(network_request, tristan::network::makeError(tristan::network::ErrorCode::DESTINATION_DIR_DOES_NOT_EXISTS));
                return;
            }
        }
        if (!network_request->m_output_file.is_open()){
            network_request->m_output_file.open(network_request->m_output_path, std::ios::ate | std::ios::binary | std::ios::app);
            if (!network_request->m_output_file.is_open()){
                pSetError(network_request, std::error_code(errno, std::system_category()));
            }
        }
        network_request->m_output_file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    }
    network_request->m_bytes_read = network_request->m_response_data->size();
}

void tristan::network::NetworkRequest::ProtectedMembers::pAddResponseData(NetworkRequest& network_request, std::vector<uint8_t>&& data){
    if (!network_request.m_output_to_file){
        if (!network_request.m_response_data){
            network_request.m_response_data = std::make_shared<std::vector<uint8_t>>(std::move(data));
        }
        else{
            network_request.m_response_data->insert(network_request.m_response_data->end(), data.begin(), data.end());
        }
    }
    else{
        if (network_request.m_output_path.empty()){
            pSetError(network_request, tristan::network::makeError(tristan::network::ErrorCode::FILE_PATH_EMPTY));
            return;
        }
        if (!std::filesystem::exists(network_request.m_output_path)){
            if (!std::filesystem::exists(network_request.m_output_path.parent_path())){
                pSetError(network_request, tristan::network::makeError(tristan::network::ErrorCode::DESTINATION_DIR_DOES_NOT_EXISTS));
                return;
            }
        }
        if (!network_request.m_output_file.is_open()){
            network_request.m_output_file.open(network_request.m_output_path, std::ios::ate | std::ios::binary | std::ios::app);
            if (!network_request.m_output_file.is_open()){
                pSetError(network_request, std::error_code(errno, std::system_category()));
            }
        }
        network_request.m_output_file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    }
    network_request.m_bytes_read = network_request.m_response_data->size();
}

void tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(const std::shared_ptr<NetworkRequest>& network_request, Status status){
    network_request->pNotifyWhenStatusChanged();

    switch (status){
        case Status::WAITING:[[fallthrough]];
        case tristan::network::Status::PROCESSED:break;
        case tristan::network::Status::PAUSED:{
            network_request->pNotifyWhenPaused();
            if (network_request->m_output_file.is_open()){
                network_request->m_output_file.close();
            }
            break;
        }
        case tristan::network::Status::RESUMED:{
            network_request->pNotifyWhenResumed();
            break;
        }
        case tristan::network::Status::ERROR:{
            network_request->pNotifyWhenFailed();
            if (network_request->m_output_file.is_open()){
                network_request->m_output_file.close();
            }
            //TODO: Decide if file should be deleted here or if download may be continued later.
        }
        case tristan::network::Status::CANCELED:{
            network_request->pNotifyWhenCanceled();
            if (network_request->m_output_file.is_open()){
                network_request->m_output_file.close();
            }
            if (std::filesystem::exists(network_request->m_output_path)){
                std::filesystem::remove(network_request->m_output_path);
            }
        }
        case tristan::network::Status::DONE:{
            network_request->pNotifyWhenFinished();
            if (network_request->m_output_file.is_open()){
                network_request->m_output_file.close();
            }
        }
    }
}

void tristan::network::NetworkRequest::ProtectedMembers::pSetStatus(NetworkRequest& network_request, Status status){
    network_request.pNotifyWhenStatusChanged();

    switch (status){
        case Status::WAITING:[[fallthrough]];
        case tristan::network::Status::PROCESSED:break;
        case tristan::network::Status::PAUSED:{
            network_request.pNotifyWhenPaused();
            if (network_request.m_output_file.is_open()){
                network_request.m_output_file.close();
            }
            break;
        }
        case tristan::network::Status::RESUMED:{
            network_request.pNotifyWhenResumed();
            break;
        }
        case tristan::network::Status::ERROR:{
            network_request.pNotifyWhenFailed();
            if (network_request.m_output_file.is_open()){
                network_request.m_output_file.close();
            }
            //TODO: Decide if file should be deleted here or if download may be continued later.
        }
        case tristan::network::Status::CANCELED:{
            network_request.pNotifyWhenCanceled();
            if (network_request.m_output_file.is_open()){
                network_request.m_output_file.close();
            }
            if (std::filesystem::exists(network_request.m_output_path)){
                std::filesystem::remove(network_request.m_output_path);
            }
        }
        case tristan::network::Status::DONE:{
            network_request.pNotifyWhenFinished();
            if (network_request.m_output_file.is_open()){
                network_request.m_output_file.close();
            }
        }
    }
}

void tristan::network::NetworkRequest::ProtectedMembers::pSetError(const std::shared_ptr<NetworkRequest>& network_request, std::error_code error_code){
    network_request->m_error = error_code;
    pSetStatus(network_request, tristan::network::Status::ERROR);
}

void tristan::network::NetworkRequest::ProtectedMembers::pSetError(NetworkRequest& network_request, std::error_code error_code){
    network_request.m_error = error_code;
    pSetStatus(network_request, tristan::network::Status::ERROR);
}