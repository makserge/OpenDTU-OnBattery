<template>
    <BasePage :title="$t('menu.RelaySettings')" :isLoading="dataLoading">
        <BootstrapAlert
            v-model="showAlert"
            dismissible
            :variant="alertType"
            :auto-dismiss="alertType === 'success' ? 5000 : 0"
        >
            {{ alertMessage }}
        </BootstrapAlert>

        <form @submit="saveSettings">
            <CardElement :text="$t('relay.GeneralSettings')" textVariant="text-bg-primary">
                <InputElement 
                    :label="$t('relay.Relay1')" 
                    v-model="config.relay1_enabled" 
                    type="checkbox" 
                    :disabled="!isConnected"
                />
                <InputElement 
                    :label="$t('relay.Relay2')" 
                    v-model="config.relay2_enabled" 
                    type="checkbox" 
                    :disabled="!isConnected"
                />
            </CardElement>

            <FormFooter @reload="getRelayData()" :is-saving="isSaving" :disabled="!isConnected" />
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
                relay1_enabled: false, 
                relay2_enabled: false 
            },
            alertMessage: '',
            alertType: 'info',
            showAlert: false
        };
    },
    created() {
        this.getRelayData();
    },
    methods: {
        async getRelayData() {
            this.dataLoading = true;
            try {
                const response = await fetch('/api/relay/status', { 
                    headers: authHeader() 
                });
                const data = await handleResponse(response, this.$emitter, this.$router);

                this.config = {
                    relay1_enabled: !!data.relay1_enabled,
                    relay2_enabled: !!data.relay2_enabled
                };
                
                this.isConnected = true;
            } catch (e) {
                this.isConnected = false;
            } finally {
                this.dataLoading = false;
            }
        },

        async saveSettings(e: Event) {
            e.preventDefault();
            this.isSaving = true;

            const formData = new FormData();
            formData.append('data', JSON.stringify(this.config));

            try {
                const response = await fetch('/api/relay/config', {
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