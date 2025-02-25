"""
URL configuration for app project.

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/5.1/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path
from . import views

urlpatterns = [
    path('admin/', admin.site.urls),
    path('abrir-porta/', views.abrir_porta_view, name='abrir_porta'),
    path('status-porta/', views.status_porta_view, name='status_porta'),
    
    path('CLIENT_CONNECTED_WH_D/', views.mqtt_client_connected, name='client_connected'),
    path('CLIENT_DISCONNECTED_WH_D/', views.mqtt_client_disconnected, name='client_disconnected'),
]