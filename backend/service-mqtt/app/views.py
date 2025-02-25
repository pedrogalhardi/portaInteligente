import json
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt
from mqtt_client_handler import get_mqtt_client

mqtt_client = get_mqtt_client()

# View para abrir a porta
def abrir_porta_view(request):
    try:
        # mqtt.abrir_porta()
        mqtt_client.publish_message("porta/comando", "OPEN")
        return JsonResponse({"status": "Comando 'OPEN' enviado com sucesso"})
    except Exception as e:
        return JsonResponse({"status": "Erro", "error": str(e)})

# View para consultar o status da porta
def status_porta_view(request):
    try:
        status = mqtt_client.get_last_message("porta/status")
        return JsonResponse({"status": status})
    except Exception as e:
        return JsonResponse({"status": "Erro", "error": str(e)})

@csrf_exempt
def mqtt_client_connected(request):
    if request.method == "POST":
        try:
            data = json.loads(request.body)
            print("connected")
            print(data)

            # if client_id:
            #     # Publica no MQTT informando que um novo cliente conectou
            #     mqtt_client.publish_message("clientes/conectados", "Novo cliente conectado")
            #     return JsonResponse({"message": "Notificação enviada com sucesso"}, status=200)
            # else:
            #     return JsonResponse({"error": "Client ID não encontrado"}, status=400)

            return JsonResponse({}, status=200)
        except json.JSONDecodeError:
            return JsonResponse({"error": "JSON inválido"}, status=400)

    return JsonResponse({"error": "Método não permitido"}, status=405)


@csrf_exempt
def mqtt_client_disconnected(request):
    if request.method == "POST":
        try:
            data = json.loads(request.body)
            print("disconnected")
            print(data)

            # if client_id:
            #     # Publica no MQTT informando que um novo cliente conectou
            #     mqtt_client.publish_message("clientes/conectados", "Novo cliente conectado")
            #     return JsonResponse({"message": "Notificação enviada com sucesso"}, status=200)
            # else:
            #     return JsonResponse({"error": "Client ID não encontrado"}, status=400)

            return JsonResponse({}, status=200)
        except json.JSONDecodeError:
            return JsonResponse({"error": "JSON inválido"}, status=400)

    return JsonResponse({"error": "Método não permitido"}, status=405)
