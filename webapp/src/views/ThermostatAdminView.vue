<template>
    <BasePage :title="$t('menu.ThermostatSettings')" :isLoading="dataLoading">
        <BootstrapAlert
            v-model="showAlert"
            dismissible
            :variant="alertType"
            :auto-dismiss="alertType === 'success' ? 5000 : 0"
        >
            {{ alertMessage }}
        </BootstrapAlert>

        <CardElement :text="$t('thermostat.CurrentStatus')" textVariant="text-bg-primary" class="mb-4">
            <div class="row align-items-center py-2">
                <div class="col-6">
                    <div class="mb-3">
                        <label class="form-label d-block text-muted small mb-0">{{ $t('thermostat.CurrentTemp') }}</label>
                        <h3 class="mb-0" :class="{'text-muted opacity-50': !isConnected}">
                            {{ isConnected ? liveData.temp.toFixed(1) + ' °C' : '--.- °C' }}
                        </h3>
                    </div>
                    <div>
                        <label class="form-label d-block text-muted small mb-0">{{ $t('thermostat.CurrentHum') }}</label>
                        <h3 class="mb-0" :class="{'text-muted opacity-50': !isConnected}">
                            {{ isConnected ? liveData.hum.toFixed(1) + ' %' : '--.- %' }}
                        </h3>
                    </div>
                </div>
                <div class="col-6 text-end">
                    <template v-if="isConnected">
                        <div v-if="liveData.heating" class="badge bg-danger d-inline-block p-2">
                            <i class="bi bi-fire me-1"></i> {{ $t('thermostat.StatusHeating') }}
                        </div>
                        <div v-else-if="liveData.cooling" class="badge bg-info text-dark d-inline-block p-2">
                            <i class="bi bi-snow me-1"></i> {{ $t('thermostat.StatusCooling') }}
                        </div>
                        <div v-else class="badge bg-secondary d-inline-block p-2">
                            {{ $t('thermostat.StatusIdle') }}
                        </div>
                    </template>
                    <div v-else class="badge bg-warning text-dark p-2">
                        <i class="bi bi-exclamation-triangle me-1"></i> {{ $t('thermostat.Disconnected') }}
                    </div>
                </div>
            </div>
        </CardElement>
        <form @submit="saveSettings">
            <CardElement :text="$t('thermostat.GeneralSettings')" textVariant="text-bg-primary">
                <InputElement 
                    :label="$t('thermostat.EnableControl')" 
                    v-model="config.enabled" 
                    type="checkbox" 
                    :disabled="!isConnected"
                />
                <InputElement
                    :label="$t('thermostat.HeaterThreshold')"
                    v-model.number="config.low_temp"
                    type="number"
                    step="0.1"
                    postfix="°C"
                    :disabled="!isConnected"
                />
                <InputElement
                    :label="$t('thermostat.CoolerThreshold')"
                    v-model.number="config.high_temp"
                    type="number"
                    step="0.1"
                    postfix="°C"
                    :disabled="!isConnected"
                />
            </CardElement>
            <FormFooter @reload="getThermostatData(true)" :is-saving="isSaving" :disabled="!isConnected" />
        </form>
    </BasePage>
</template>

<script lang="ts">
import { defineComponent } from 'vue';
import BasePage from '@/components/BasePage.vue';
import BootstrapAlert from '@/components/BootstrapAlert.vue';
import CardElement from '@/components/CardElement.vue';
import FormFooter from '@/components/FormFooter.vue';
import InputElement from '@/components/InputElement.vue';
import { authHeader, handleResponse } from '@/utils/authentication';

export default defineComponent({
    components: {
        BasePage,
        BootstrapAlert,
        CardElement,
        FormFooter,
        InputElement,
    },
    data() {
        return {
            dataLoading: true,
            isSaving: false,
            isConnected: true,
            // config holds user input, separated from live status to prevent polling resets
            config: { 
                enabled: false, 
                low_temp: 20.0, 
                high_temp: 25.0 
            },
            liveData: { 
                temp: 0.0,
                hum: 0.0,
                heating: false, 
                cooling: false 
            },
            alertMessage: '',
            alertType: 'info',
            showAlert: false,
            pollTimer: null as number | null,
        };
    },
    created() {
        this.getThermostatData(true);
        this.pollTimer = window.setInterval(() => this.getThermostatData(false), 3000);
    },
    beforeUnmount() {
        if (this.pollTimer) clearInterval(this.pollTimer);
    },
    methods: {
        async getThermostatData(isInitial = false) {
            if (isInitial) this.dataLoading = true;
            try {
                const response = await fetch('/api/thermostat/status', { 
                    headers: authHeader() 
                });
                const data = await handleResponse(response, this.$emitter, this.$router);

                this.liveData = {
                    temp: data.temp ?? this.liveData.temp,
                    hum: data.hum ?? this.liveData.hum,
                    heating: !!data.heating,
                    cooling: !!data.cooling
                };

                if (isInitial) {
                    this.config = {
                        enabled: !!data.enabled,
                        low_temp: data.low_temp,
                        high_temp: data.high_temp
                    };
                }
                this.isConnected = true;
            } catch (e) {
                this.isConnected = false;
            } finally {
                if (isInitial) this.dataLoading = false;
            }
        },

        async saveSettings(e: Event) {
            e.preventDefault();
            this.isSaving = true;

            const formData = new FormData();
            formData.append('data', JSON.stringify(this.config));

            try {
                const response = await fetch('/api/thermostat/config', {
                    method: 'POST',
                    headers: authHeader(),
                    body: formData,
                });
                
                const result = await handleResponse(response, this.$emitter, this.$router);
                
                this.alertMessage = this.$t('apiresponse.' + result.code, result.param);
                this.alertType = result.type;
                this.showAlert = true;
                
                window.scrollTo(0, 0);
            } catch (e) {
                console.error("Save error", e);
            } finally {
                this.isSaving = false;
            }
        }
    },
});
</script>