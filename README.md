# Estacion-Meteorologica-Automatica
Este proyecto es un sistema de monitoreo ambiental que utiliza sensores para medir temperatura, humedad, luz, UV y humedad del suelo. Con tecnología LoRa, un receptor y dos transmisores, recopila datos, los almacena en una SD y los envía a la nube vía ThingSpeak, siendo eficiente y adecuado para entornos remotos y agrícolas.
Características principales:
1. Transmisión de datos inalámbrica con LoRa: Se utilizan transmisores para enviar datos a un receptor central, garantizando una comunicación confiable y de largo alcance.
2. Monitoreo ambiental en tiempo real: Incluye sensores como el DHT22, BH1750 y sensores de humedad del suelo, que recopilan datos sobre temperatura, humedad, luz y calidad del suelo.
3. Gestión y almacenamiento de datos: Utiliza una tarjeta SD para el almacenamiento local, junto con la integración de ThingSpeak para subir los datos a la nube para análisis y visualización.
4. Automatización y eficiencia energética: Implementa el modo deep sleep en el microcontrolador ESP32, optimizando el consumo energético.
5. Compatibilidad con plataformas IoT: Gracias a la integración con ThingSpeak y la programación en C++, se habilita el análisis remoto y la automatización del sistema.
![WhatsApp Image 2024-09-04 at 9 57 10 PM (2)](https://github.com/user-attachments/assets/406d90e1-c03f-4515-8ba4-ff6a0205199c)
