<template>
    <BasePage :title="$t('menu.PowerMeterBlSettings')" :isLoading="dataLoading">
        <BootstrapAlert
            v-model="showAlert"
            dismissible
            :variant="alertType"
            :auto-dismiss="alertType === 'success' ? 5000 : 0"
        >
            {{ alertMessage }}
        </BootstrapAlert>

        <CardElement :text="$t('powermeterbl.CurrentStatus')" textVariant="text-bg-primary" table>
            <div class="table-responsive">
                <table class="table table-hover table-condensed">
                    <tbody>
                        <tr>
                            <th>{{ $t('powermeterbl.Power') }}</th>
                            <td>{{ liveData.power }}</td>
                        </tr>
                        <tr>
                            <th>{{ $t('powermeterbl.ApparentPower') }}</th>
                            <td>{{ liveData.apparent_power }}</td>
                        </tr>
                        <tr>
                            <th>{{ $t('powermeterbl.Current') }}</th>
                            <td>{{ liveData.current }}</td>
                        </tr>
                        <tr>
                            <th>{{ $t('powermeterbl.Voltage') }}</th>
                            <td>{{ liveData.voltage }}</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </CardElement>

        <form @submit="saveSettings">
            <CardElement :text="$t('powermeterbl.GeneralSettings')" textVariant="text-bg-primary">
                <InputElement 
                    :label="$t('powermeterbl.EnableControl')" 
                    v-model="config.enabled" 
                    type="checkbox" 
                    :disabled="!isConnected"
                />
            </CardElement>

            <FormFooter @reload="getPowerMeterData(true)" :is-saving="isSaving" :disabled="!isConnected" />
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
            config: { 
                enabled: false, 
            },
            liveData: { 
                power: 0, 
                apparent_power: 0, 
                voltage: 0.0,
                current: 0.0
            },
            alertMessage: '',
            alertType: 'info',
            showAlert: false,
            pollTimer: null as number | null,
        };
    },
    created() {
        this.getPowerMeterData(true);
        this.pollTimer = window.setInterval(() => this.getPowerMeterData(false), 2000);
    },
    beforeUnmount() {
        if (this.pollTimer) clearInterval(this.pollTimer);
    },
    methods: {
        async getPowerMeterData(isInitial = false) {
            if (isInitial) this.dataLoading = true;
            try {
                const response = await fetch('/api/powermeter_bl/status', { 
                    headers: authHeader() 
                });
                const data = await handleResponse(response, this.$emitter, this.$router);

                this.liveData = {
                    power: data.power ?? this.liveData.power,
                    apparent_power: data.apparent_power ?? this.liveData.apparent_power,
                    voltage: data.voltage ?? this.liveData.voltage,
                    current: data.current ?? this.liveData.current
                };

                if (isInitial) {
                    this.config = {
                        enabled: !!data.enabled,
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
                const response = await fetch('/api/powermeter_bl/config', {
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