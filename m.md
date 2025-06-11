```mermaid
stateDiagram-v2
    [*] --> Ready: xTaskCreate()
    Ready --> Running
    Running --> Blocked: xSemaphoreTake()
    Blocked --> Ready: xSemaphoreGive()
    Running --> Suspended: vTaskDelete()
    Suspended --> [*]
